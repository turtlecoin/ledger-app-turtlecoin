// Copyright (c) 2018-2020, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

import Transport, { Observer, Subscription } from '@ledgerhq/hw-transport';
import { Reader, Writer } from '@turtlecoin/bytestream';
import { Socket, createConnection } from 'net';
import { AbortController } from 'abort-controller/dist/abort-controller';

export class TCPTransport extends Transport<string> {
    private readonly m_socket: Socket;
    private readonly m_timeout: number;
    private m_verbose = false;
    private m_scrambleKey?: string;

    constructor (socket: Socket, timeout = 30000) {
        super();

        this.m_socket = socket;

        this.m_socket.on('error', async () => {
            await this.close();
        });

        this.m_socket.on('data', (data: Buffer) => {
            if (this.verbose) {
                console.log('<- %s', data.toString('hex'));
            }
        });

        this.m_timeout = timeout;

        this.m_socket.setTimeout(timeout);
    }

    public get verbose (): boolean {
        return this.m_verbose;
    }

    public set verbose (val: boolean) {
        this.m_verbose = val;
    }

    public static async isSupported (): Promise<boolean> {
        return true;
    }

    public static async list (): Promise<any[]> {
        return [];
    }

    public static listen (_obs: Observer<any>): Subscription {
        return {
            unsubscribe: () => { _obs.complete(); }
        };
    }

    public static async open (host: string, timeout = 30000): Promise<TCPTransport> {
        return new Promise((resolve, reject) => {
            const [ip, port] = host.split(':', 2);

            const socket = createConnection({ port: parseInt(port, 10), host: ip, timeout });

            socket.once('connect', () => {
                const instance = new TCPTransport(socket, timeout);

                return resolve(instance);
            });

            socket.once('error', error => {
                return reject(error);
            });
        });
    }

    public setScrambleKey (key?: string) {
        this.m_scrambleKey = key;
    }

    public async close (): Promise<void> {
        return new Promise(resolve => {
            this.m_socket.end(() => {
                return resolve();
            });
        });
    }

    public async exchange (apdu: Buffer): Promise<Buffer> {
        return new Promise((resolve, reject) => {
            const controller = new AbortController();

            controller.signal.addEventListener('abort', () => {
                return reject(new Error('exchange process timed out'));
            });

            const timeout = setTimeout(() => controller.abort(), this.m_timeout);

            const writer = new Writer();

            writer.uint32_t(apdu.length, true);
            writer.write(apdu);

            this.m_socket.once('data', data => {
                const reader = new Reader(data);

                const size = reader.uint32_t(true).toJSNumber();

                if (reader.unreadBytes !== size + 2) {
                    return reject(new Error('Payload size does not match expected size'));
                }

                const response = reader.unreadBuffer;

                if (size > 0) {
                    reader.skip(size);
                }

                const code = reader.uint16_t(true).toJSNumber();
                if (code === 0x9000) {
                    clearTimeout(timeout);

                    return resolve(response);
                } else if ((code & 0xff00) !== 0x6100) {
                    return reject(new Error('Invalid status code supplied'));
                } else {
                    return reject(new Error('Unhandled response'));
                }
            });

            send(this.m_socket, writer.buffer, this.verbose);
        });
    }
}

async function send (socket: Socket, data: Buffer, verbose = false): Promise<void> {
    return new Promise((resolve, reject) => {
        if (verbose) {
            console.log('-> %s', data.toString('hex'));
        }

        socket.write(data, error => {
            if (error) {
                return reject(error);
            }

            return resolve();
        });
    });
}
