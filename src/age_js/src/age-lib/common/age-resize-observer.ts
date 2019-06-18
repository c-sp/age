//
// Copyright 2018 Christoph Sprenger
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

import {ElementRef} from "@angular/core";
import ResizeObserver from "resize-observer-polyfill";
import {AgeSubscriptionLike} from "./age-subscription-like";


export class AgeResizeObserver extends AgeSubscriptionLike {

    private readonly _resizeObserver: ResizeObserver;

    constructor(elementToObserve: ElementRef,
                resizeCallback: (entry: ResizeObserverEntry) => void) {

        super(() => this._resizeObserver.disconnect());

        this._resizeObserver = new ResizeObserver(entries => {
            // we observe a single element and thus use only the first entry
            const entry = (entries && entries.length) ? entries[0] : undefined;
            if (entry) {
                resizeCallback(entry);
            }
        });

        this._resizeObserver.observe(elementToObserve.nativeElement);
    }
}
