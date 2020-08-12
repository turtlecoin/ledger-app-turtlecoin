// Copyright (c) 2018-2020, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

import { Address, Crypto, LedgerDevice } from 'turtlecoin-utils';
import { before, describe, it } from 'mocha';
import { TCPTransport } from './TCPTransport';
import Transport from '@ledgerhq/hw-transport';
import * as assert from 'assert';

/** @ignore */
const confirm = !!(process.env.CONFIRM && process.env.CONFIRM.length !== 0);

/**
 * This set of tests is designed to test the application and thus the crypto operations
 * provided by the TurtleCoin application running on a Ledger hardware device against
 * the TurtleCoin Crypto library that is a known working package for the same underlying
 * functions. As the TurtleCoin Crypto library is used throughout numerous wallets including
 * the GUI wallet & mobile wallet, testing against the results provided by that library
 * via that library gives us a set of known good results that we should expect to return
 * from the ledger device regardless of any random scalars used and/or created during
 * many of the stealth operations inherent to the CryptoNote protocol
 */

describe('Ledger Hardware Tests', function () {
    this.timeout(120000); // some of these tests can take a VERY long time

    /**
     * This is the known wallet seed that is generated based upon the 24 word word
     * mnemonic that is provided to Speculos upon launching the TurtleCoin ledger
     * application for testing purposes. This is revealed such that we can use the
     * TurtleCoin Crypto library to perform our checks using the exact same set
     * of wallet keys
     */
    const walletSeed = '74e4ac6f5a858c4161593a90d2f6f22d3a57195a89e75d10500d68db3c68c70f';

    const ledgerIdent = '547572746c65436f696e206973206e6f742061204d6f6e65726f20666f726b21';

    let ledger: LedgerDevice;
    let transport: Transport;
    let Wallet: Address;

    const TurtleCoinCrypto = new Crypto();

    before(async () => {
        transport = await TCPTransport.open('127.0.0.1:9999');

        ledger = new LedgerDevice(transport);

        Wallet = await Address.fromSeed(walletSeed);
    });

    after(async () => {
        if (transport) {
            await transport.close();
        }
    });

    describe('General Functions', () => {
        it('Version', async () => {
            return ledger.getVersion();
        });

        it('Ident', async () => {
            const result = await ledger.getIdent();

            assert(result === ledgerIdent);
        });

        it('Is Debug?', async () => {
            assert(await ledger.isDebug());
        });
    });

    describe('Key Fundamentals', () => {
        it('Generate Random Key Pair', async () => {
            const keys = await ledger.getRandomKeyPair();

            assert(await TurtleCoinCrypto.checkKey(keys.publicKey) &&
                await TurtleCoinCrypto.checkScalar(keys.privateKey));
        });

        it('Private Key to Public Key', async () => {
            const keys = await TurtleCoinCrypto.generateKeys();

            const result = await ledger.privateToPublic(keys.privateKey);

            assert(result.publicKey === keys.publicKey);
        });

        it('Private Key to Public Key: Supplying public key fails', async () => {
            const keys = await TurtleCoinCrypto.generateKeys();

            await ledger.privateToPublic(keys.publicKey)
                .then(() => assert(false))
                .catch(() => assert(true));
        });

        it('Get Public Keys', async () => {
            const keys = await ledger.getPublicKeys(confirm);

            assert(keys.spend.publicKey === Wallet.spend.publicKey && keys.view.publicKey === Wallet.view.publicKey);
        });

        it('Get Private Spend Key', async () => {
            const key = await ledger.getPrivateSpendKey(confirm);

            assert(key.privateKey === Wallet.spend.privateKey);
        });

        it('Get Private View Key', async () => {
            const key = await ledger.getPrivateViewKey(confirm);

            assert(key.privateKey === Wallet.view.privateKey);
        });

        it('Get Wallet Address', async () => {
            const result = await ledger.getAddress(confirm);

            assert(await result.address() === await Wallet.address());
        });

        it('Check Key', async () => {
            assert(await ledger.checkKey(Wallet.spend.publicKey));
        });

        it('Check Key: Fails on non-point', async () => {
            assert(!await ledger.checkKey(Wallet.spend.privateKey));
        });

        it('Check Scalar', async () => {
            assert(await ledger.checkScalar(Wallet.spend.privateKey));
        });

        it('Check Scalar: Fails on non-scalar', async () => {
            assert(!await ledger.checkScalar(Wallet.spend.publicKey));
        });

        it('Reset Keys', async () => {
            return ledger.resetKeys(confirm);
        });
    });

    describe('Signing Fundamentals', () => {
        let message_digest: string;

        before(async () => {
            message_digest = await TurtleCoinCrypto.cn_fast_hash(ledgerIdent);
        });

        it('Generate Signature', async () => {
            const signature = await ledger.generateSignature(message_digest, confirm);

            assert(await TurtleCoinCrypto.checkSignature(message_digest, Wallet.spend.publicKey, signature));
        });

        it('Check Signature', async () => {
            const signature = await TurtleCoinCrypto.generateSignature(
                message_digest, Wallet.spend.publicKey, Wallet.spend.privateKey);

            assert(await ledger.checkSignature(message_digest, Wallet.spend.publicKey, signature));
        });

        it('Check Signature: Supplying private key fails', async () => {
            const signature = await TurtleCoinCrypto.generateSignature(
                message_digest, Wallet.spend.publicKey, Wallet.spend.privateKey);

            await ledger.checkSignature(message_digest, Wallet.spend.privateKey, signature)
                .then(() => assert(false))
                .catch(() => assert(true));
        });

        it('Check Signature: Supplying invalid signature fails', async () => {
            const signature = await TurtleCoinCrypto.generateSignature(
                message_digest, Wallet.spend.publicKey, Wallet.spend.privateKey);

            await ledger.checkSignature(message_digest, Wallet.spend.publicKey, signature.split('').reverse().join(''))
                .then(() => assert(false))
                .catch(() => assert(true));
        });
    });

    describe('Stealth Operations', () => {
        let ready = false;
        const output_index = 2;

        let tx_public_key: string;
        let expected_derivation: string;
        let expected_publicEphemeral: string;
        let expected_privateEphemeral: string;
        let expected_key_image: string;

        before(async () => {
            tx_public_key = (await TurtleCoinCrypto.generateKeys()).publicKey;

            expected_derivation = await TurtleCoinCrypto.generateKeyDerivation(
                tx_public_key, Wallet.view.privateKey);

            expected_publicEphemeral = await TurtleCoinCrypto.derivePublicKey(
                expected_derivation, output_index, Wallet.spend.publicKey);

            expected_privateEphemeral = await TurtleCoinCrypto.deriveSecretKey(
                expected_derivation, output_index, Wallet.spend.privateKey);

            expected_key_image = await TurtleCoinCrypto.generateKeyImage(
                expected_publicEphemeral, expected_privateEphemeral);

            ready = true;
        });

        it('Generate Key Derivation', async () => {
            const derivation = await ledger.generateKeyDerivation(tx_public_key, confirm);

            assert(expected_derivation === derivation);
        });

        it('Generate Key Derivation: Fails when supplying private key', async () => {
            await ledger.generateKeyDerivation(expected_privateEphemeral, confirm)
                .then(() => assert(false))
                .catch(() => assert(true));
        });

        it('Derive Public Key', async () => {
            const result = await ledger.derivePublicKey(expected_derivation, output_index, confirm);

            assert(result.publicKey === expected_publicEphemeral);
        });

        it('Derive Public Key: Fails when wrong output index', async () => {
            const result = await ledger.derivePublicKey(expected_derivation, output_index + 3, confirm);

            assert(result.publicKey !== expected_publicEphemeral);
        });

        it('Derive Public Key: Fails when wrong derivation supplied', async () => {
            const result = await ledger.derivePublicKey(tx_public_key, output_index, confirm);

            assert(result.publicKey !== expected_publicEphemeral);
        });

        it('Derive Secret Key', async () => {
            const result = await ledger.deriveSecretKey(expected_derivation, output_index, confirm);

            assert(result.privateKey === expected_privateEphemeral);
        });

        it('Derive Secret Key: Fails when wrong output index', async () => {
            const result = await ledger.deriveSecretKey(expected_derivation, output_index + 3, confirm);

            assert(result.privateKey !== expected_privateEphemeral);
        });

        it('Derive Secret Key: Fails when wrong derivation supplied', async () => {
            const result = await ledger.deriveSecretKey(tx_public_key, output_index, confirm);

            assert(result.privateKey !== expected_privateEphemeral);
        });

        it('Generate Key Image', async () => {
            const key_image = await ledger.generateKeyImage(
                tx_public_key, output_index, expected_publicEphemeral, confirm);

            assert(key_image === expected_key_image);
        });

        it('Generate Key Image: Fails when wrong output index', async () => {
            await ledger.generateKeyImage(
                tx_public_key, output_index + 3, expected_publicEphemeral, confirm)
                .then(() => assert(false))
                .catch(() => assert(true));
        });

        describe('Ring Signatures', () => {
            const public_keys: string[] = [];
            const real_output_index = 0;

            let expected_ring_signatures: string[];
            let prepared_ring_signatures: string[];
            let tx_prefix_hash: string;
            let k: string;

            before(async () => {
                const wait = () => {
                    return new Promise(resolve => {
                        const check = () => {
                            if (!ready) {
                                setTimeout(check, 100);
                            } else {
                                return resolve();
                            }
                        };
                        check();
                    });
                };
                await wait();

                public_keys.push(expected_publicEphemeral);

                tx_prefix_hash = await TurtleCoinCrypto.cn_fast_hash(ledgerIdent);

                for (let i = 0; i < 3; i++) {
                    public_keys.push((await TurtleCoinCrypto.generateKeys()).publicKey);
                }

                const prepped = await TurtleCoinCrypto.prepareRingSignatures(
                    tx_prefix_hash, expected_key_image, public_keys, real_output_index);

                prepared_ring_signatures = prepped.signatures;

                k = prepped.key;

                expected_ring_signatures = await TurtleCoinCrypto.generateRingSignatures(
                    tx_prefix_hash, expected_key_image, public_keys, expected_privateEphemeral, real_output_index);
            });

            it('Complete Ring Signatures', async function () {
                const signatures = prepared_ring_signatures;

                signatures[real_output_index] = await ledger.completeRingSignature(
                    tx_public_key, output_index, expected_publicEphemeral, k, signatures[real_output_index], confirm);

                assert(await TurtleCoinCrypto.checkRingSignature(
                    tx_prefix_hash, expected_key_image, public_keys, signatures));
            });

            it('Complete Ring Signatures: Fails when wrong output index', async function () {
                await ledger.completeRingSignature(
                    tx_public_key, output_index + 3, expected_publicEphemeral, k, prepared_ring_signatures[real_output_index], confirm)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });

            it('Generate Ring Signatures', async function () {
                const sigs = await ledger.generateRingSignatures(
                    tx_public_key, output_index, expected_publicEphemeral, tx_prefix_hash,
                    public_keys, real_output_index, confirm);

                assert(await TurtleCoinCrypto.checkRingSignatures(
                    tx_prefix_hash, expected_key_image, public_keys, sigs));
            });

            it('Generate Ring Signatures: Fails when wrong output index', async function () {
                await ledger.generateRingSignatures(
                    tx_public_key, output_index + 3, expected_publicEphemeral, tx_prefix_hash,
                    public_keys, real_output_index, confirm)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });

            it('Check Ring Signatures', async function () {
                assert(await ledger.checkRingSignatures(
                    tx_prefix_hash, expected_key_image, public_keys, expected_ring_signatures));
            });

            it('Check Ring Signatures: Fails when wrong signatures supplied', async function () {
                assert(!await ledger.checkRingSignatures(
                    tx_prefix_hash, expected_key_image, public_keys, expected_ring_signatures.reverse()));
            });
        });
    });

    describe('Transaction Construction Tests', function () {
        let skipTests = false;

        let cancelTests = false;

        let tx_public_key: string;

        let payment_id: string;

        before('start tx', async function () {
            try {
                const state = await ledger.transactionState();

                if (state === 0) {
                    const keys = await TurtleCoinCrypto.generateKeys();

                    tx_public_key = keys.publicKey;

                    const output_keys = await TurtleCoinCrypto.generateKeys();

                    payment_id = output_keys.privateKey;

                    await ledger.startTransaction(3000000, 2, 1, tx_public_key, payment_id);
                } else {
                    skipTests = true;
                    this.skip();
                }
            } catch (e) {
                skipTests = true;
                this.skip();
            }
        });

        after('reset tx', async () => {
            if (!skipTests) {
                await ledger.resetTransaction(confirm);
            }
        });

        describe('Construct Transaction on Device', () => {
            let input_tx_public_key: string;

            const input_output_index = 0;

            const mixins: string[] = [];

            let output_key: string;

            let tx_hash: string;

            let tx_size: number;

            before('Create seed data', async () => {
                /**
                 * The vast majority of this data is randomly generated
                 * data that can be used to construct a "fake" transaction
                 * that follows the correct methods to create a "real" transaction
                 */
                const keys = await TurtleCoinCrypto.generateKeys();

                input_tx_public_key = keys.publicKey;

                const derivation = await TurtleCoinCrypto.generateKeyDerivation(
                    input_tx_public_key, Wallet.view.privateKey);

                const publicEphemeral = await TurtleCoinCrypto.derivePublicKey(
                    derivation, input_output_index, Wallet.spend.publicKey);

                mixins.push(publicEphemeral);

                for (let i = 0; i < 3; i++) {
                    const mixin = await TurtleCoinCrypto.generateKeys();

                    mixins.push(mixin.publicKey);
                }

                const output = await TurtleCoinCrypto.generateKeys();

                output_key = output.publicKey;
            });

            it('Check state: Ready', async () => {
                const state = await ledger.transactionState();

                if (state !== 1) {
                    cancelTests = true;
                }

                assert(state === 1);
            });

            it('Initiate Input Loading', async function () {
                if (cancelTests) {
                    return this.skip();
                }

                await ledger.startTransactionInputLoad()
                    .catch(() => {
                        cancelTests = true;

                        assert(false);
                    });
            });

            it('Check state: Input Load #1', async function () {
                if (cancelTests) {
                    return this.skip();
                }

                const state = await ledger.transactionState();

                if (state !== 2) {
                    cancelTests = true;
                }

                assert(state === 2);
            });

            it('Load input', async function () {
                if (cancelTests) {
                    return this.skip();
                }

                await ledger.loadTransactionInput(
                    input_tx_public_key, input_output_index, 54321, mixins, [0, 1, 2, 3], 0)
                    .catch(() => {
                        cancelTests = true;

                        assert(false);
                    });
            });

            it('Fail to load input that does not belong to us', async function () {
                if (cancelTests) {
                    return this.skip();
                }

                await ledger.loadTransactionInput(
                    input_tx_public_key, input_output_index, 12345, mixins, [0, 1, 2, 3], 1)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });

            it('Check state: Input Load #2', async function () {
                if (cancelTests) {
                    return this.skip();
                }

                const state = await ledger.transactionState();

                if (state !== 2) {
                    cancelTests = true;
                }

                assert(state === 2);
            });

            it('Load input', async function () {
                if (cancelTests) {
                    return this.skip();
                }

                await ledger.loadTransactionInput(
                    input_tx_public_key, input_output_index, 12345, mixins, [0, 1, 2, 3], 0)
                    .catch(() => {
                        cancelTests = true;

                        assert(false);
                    });
            });

            it('Check state: Inputs Received', async function () {
                if (cancelTests) {
                    return this.skip();
                }

                const state = await ledger.transactionState();

                if (state !== 3) {
                    cancelTests = true;
                }

                assert(state === 3);
            });

            it('Initiate Output Loading', async function () {
                if (cancelTests) {
                    return this.skip();
                }

                await ledger.startTransactionOutputLoad()
                    .catch(() => {
                        cancelTests = true;

                        assert(false);
                    });
            });

            it('Load output', async function () {
                if (cancelTests) {
                    return this.skip();
                }

                await ledger.loadTransactionOutput(5000, output_key)
                    .catch(() => {
                        cancelTests = true;

                        assert(false);
                    });
            });

            it('Check state: Outputs Received', async function () {
                if (cancelTests) {
                    return this.skip();
                }

                const state = await ledger.transactionState();

                if (state !== 5) {
                    cancelTests = true;
                }

                assert(state === 5);
            });

            it('Finalize Transaction', async function () {
                if (cancelTests) {
                    return this.skip();
                }

                await ledger.finalizeTransactionPrefix()
                    .catch(() => {
                        cancelTests = true;

                        assert(false);
                    });
            });

            it('Check state: Prefix Ready', async function () {
                if (cancelTests) {
                    return this.skip();
                }

                const state = await ledger.transactionState();

                if (state !== 6) {
                    cancelTests = true;
                }

                assert(state === 6);
            });

            it('Sign Transaction', async function () {
                if (cancelTests) {
                    return this.skip();
                }

                await ledger.signTransaction(confirm)
                    .then(result => {
                        tx_hash = result.hash;

                        tx_size = result.size;
                    })
                    .catch(() => {
                        cancelTests = true;

                        assert(false);
                    });
            });

            it('Check state: Complete', async function () {
                if (cancelTests) {
                    return this.skip();
                }

                const state = await ledger.transactionState();

                if (state !== 7) {
                    cancelTests = true;
                }

                assert(state === 7);
            });

            it('Retrieve Transaction', async function () {
                if (cancelTests) {
                    return this.skip();
                }

                const transaction = await ledger.retrieveTransaction();

                assert(tx_size === transaction.size);
                assert(tx_hash === await transaction.hash());
                assert(payment_id === transaction.paymentId);
            });
        });

        describe('Key Fundamentals Fail While in Transaction State', () => {
            it('Generate Random Key Pair', async () => {
                await ledger.getRandomKeyPair()
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });

            it('Private Key to Public Key', async () => {
                const keys = await TurtleCoinCrypto.generateKeys();

                await ledger.privateToPublic(keys.privateKey)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });

            it('Get Public Keys', async () => {
                await ledger.getPublicKeys(confirm)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });

            it('Get Private Spend Key', async () => {
                await ledger.getPrivateSpendKey(confirm)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });

            it('Get Private View Key', async () => {
                await ledger.getPrivateViewKey(confirm)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });

            it('Get Wallet Address', async () => {
                await ledger.getAddress(confirm)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });

            it('Check Key', async () => {
                await ledger.checkKey(Wallet.spend.publicKey)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });

            it('Check Scalar', async () => {
                await ledger.checkScalar(Wallet.spend.privateKey)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });

            it('Reset Keys', async () => {
                return ledger.resetKeys(confirm)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });
        });

        describe('Signing Fundamentals Fail While in Transaction State', () => {
            let message_digest: string;

            before(async () => {
                message_digest = await TurtleCoinCrypto.cn_fast_hash(ledgerIdent);
            });

            it('Generate Signature', async () => {
                await ledger.generateSignature(message_digest, confirm)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });

            it('Check Signature', async () => {
                const signature = await TurtleCoinCrypto.generateSignature(
                    message_digest, Wallet.spend.publicKey, Wallet.spend.privateKey);

                await ledger.checkSignature(message_digest, Wallet.spend.publicKey, signature)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });
        });

        describe('Stealth Operations Fail While in Transaction State', () => {
            let ready = false;
            const output_index = 2;

            let tx_public_key: string;
            let expected_derivation: string;
            let expected_publicEphemeral: string;
            let expected_privateEphemeral: string;
            let expected_key_image: string;

            before(async () => {
                tx_public_key = (await TurtleCoinCrypto.generateKeys()).publicKey;

                expected_derivation = await TurtleCoinCrypto.generateKeyDerivation(
                    tx_public_key, Wallet.view.privateKey);

                expected_publicEphemeral = await TurtleCoinCrypto.derivePublicKey(
                    expected_derivation, output_index, Wallet.spend.publicKey);

                expected_privateEphemeral = await TurtleCoinCrypto.deriveSecretKey(
                    expected_derivation, output_index, Wallet.spend.privateKey);

                expected_key_image = await TurtleCoinCrypto.generateKeyImage(
                    expected_publicEphemeral, expected_privateEphemeral);

                ready = true;
            });

            it('Generate Key Derivation', async () => {
                await ledger.generateKeyDerivation(tx_public_key, confirm)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });

            it('Derive Public Key', async () => {
                await ledger.derivePublicKey(expected_derivation, output_index, confirm)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });

            it('Derive Secret Key', async () => {
                await ledger.deriveSecretKey(expected_derivation, output_index, confirm)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });

            it('Generate Key Image', async () => {
                await ledger.generateKeyImage(
                    tx_public_key, output_index, expected_publicEphemeral, confirm)
                    .then(() => assert(false))
                    .catch(() => assert(true));
            });

            describe('Ring Signatures Fail While in Transaction State', () => {
                const public_keys: string[] = [];
                const real_output_index = 0;

                let expected_ring_signatures: string[];
                let prepared_ring_signatures: string[];
                let tx_prefix_hash: string;
                let k: string;

                before(async () => {
                    const wait = () => {
                        return new Promise(resolve => {
                            const check = () => {
                                if (!ready) {
                                    setTimeout(check, 100);
                                } else {
                                    return resolve();
                                }
                            };
                            check();
                        });
                    };
                    await wait();

                    public_keys.push(expected_publicEphemeral);

                    tx_prefix_hash = await TurtleCoinCrypto.cn_fast_hash(ledgerIdent);

                    for (let i = 0; i < 3; i++) {
                        public_keys.push((await TurtleCoinCrypto.generateKeys()).publicKey);
                    }

                    const prepped = await TurtleCoinCrypto.prepareRingSignatures(
                        tx_prefix_hash, expected_key_image, public_keys, real_output_index);

                    prepared_ring_signatures = prepped.signatures;

                    k = prepped.key;

                    expected_ring_signatures = await TurtleCoinCrypto.generateRingSignatures(
                        tx_prefix_hash, expected_key_image, public_keys, expected_privateEphemeral, real_output_index);
                });

                it('Complete Ring Signatures', async function () {
                    ledger.completeRingSignature(
                        tx_public_key, output_index, expected_publicEphemeral, k, prepared_ring_signatures[real_output_index], confirm)
                        .then(() => assert(false))
                        .catch(() => assert(true));
                });

                it('Generate Ring Signatures', async function () {
                    await ledger.generateRingSignatures(
                        tx_public_key, output_index, expected_publicEphemeral, tx_prefix_hash,
                        public_keys, real_output_index, false)
                        .then(() => assert(false))
                        .catch(() => assert(true));
                });

                it('Check Ring Signatures', async function () {
                    await ledger.checkRingSignatures(
                        tx_prefix_hash, expected_key_image, public_keys, expected_ring_signatures)
                        .then(() => assert(false))
                        .catch(() => assert(true));
                });
            });
        });
    });

    describe('Methods restored when Transaction State reset', () => {
        it('Generate Random Key Pair', async () => {
            const keys = await ledger.getRandomKeyPair();

            assert(await TurtleCoinCrypto.checkKey(keys.publicKey) &&
                await TurtleCoinCrypto.checkScalar(keys.privateKey));
        });
    });
});
