//
// Copyright 2020 Christoph Sprenger
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
import {IncomingHttpHeaders} from 'http';
import {request, RequestOptions} from 'https';
import {Observable, of} from 'rxjs';
import {switchMap} from 'rxjs/operators';
import {parse} from 'url';


export interface IHttpsResponse {
    readonly statusCode?: number;
    readonly headers: IncomingHttpHeaders;
    readonly data: Buffer;
}

export function simpleHttpsRequest$(options: RequestOptions): Observable<IHttpsResponse> {
    return new Observable(subscriber => {
        const req = request(
            options,
            res => {
                // read response
                const buffers = new Array<Buffer>();
                res.on('data', data => buffers.push(data));

                // forward response to subscriber
                res.on('end', () => {
                    const data = Buffer.concat(buffers);
                    subscriber.next({
                        statusCode: res.statusCode,
                        headers: res.headers,
                        data,
                    });
                });
            },
        );

        // on error notify subscriber
        req.on(
            'error',
            err => subscriber.error(err),
        );

        // finish sending the request
        req.end();

        // return teardown logic
        return () => req.abort();
    });
}


export function httpRequest$(options: RequestOptions): Observable<IHttpsResponse> {
    return simpleHttpsRequest$(options).pipe(
        switchMap(httpsResponse => {
            const statusCode = httpsResponse.statusCode || 0;

            // successful request
            if ((statusCode >= 200) && (statusCode < 300)) {
                return of(httpsResponse);
            }

            // redirect
            if ((statusCode >= 300) && (statusCode < 400)) {
                const redirectLocation = httpsResponse.headers.location;
                if (!redirectLocation) {
                    throw new Error('redirect without location header');
                }

                // the location header might contain just the path without the hostname
                const redirectUrl = parse(redirectLocation);
                const newOptions = {
                    ...options,
                    host: redirectUrl.host || options.host,
                    path: redirectUrl.path,
                };

                return httpRequest$(newOptions);
            }

            // we treat any other status code as error
            throw new Error(`${options.method} ${options.host}${options.path} ${httpsResponse.statusCode}`);
        }),
    );
}
