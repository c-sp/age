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

import {ElementRef, NgZone} from '@angular/core';
import ResizeObserver from 'resize-observer-polyfill';
import {combineLatest, Observable, Subscriber} from 'rxjs';
import {map} from 'rxjs/operators';
import {AgeScreenSize} from '../../emulation';


export interface IAgeViewport {
    readonly left: number;
    readonly top: number;
    readonly width: number;
    readonly height: number;
}

export function emulationViewport$(viewportContainer: ElementRef,
                                   screenSize$: Observable<AgeScreenSize | undefined>,
                                   ngZone: NgZone): Observable<IAgeViewport> {

    return combineLatest([
        observeElementSize$(viewportContainer, ngZone),
        screenSize$,
    ]).pipe(
        map(values => calculateViewportSize(values[0], values[1])),
    );
}


function observeElementSize$(elementToObserve: ElementRef,
                             ngZone: NgZone): Observable<ResizeObserverEntry> {

    return new Observable<ResizeObserverEntry>((subscriber: Subscriber<ResizeObserverEntry>) => {

        const resizeObserver = new ResizeObserver(entries => {
            // we observe a single element and thus use only the first entry
            const entry = (entries && entries.length) ? entries[0] : undefined;
            if (entry) {
                ngZone.run(() => subscriber.next(entry));
            }
        });
        resizeObserver.observe(elementToObserve.nativeElement);

        return () => resizeObserver.disconnect();
    });
}


function calculateViewportSize(resizeEntry: ResizeObserverEntry,
                               screenSize: AgeScreenSize | undefined): IAgeViewport {

    const screenWidth = (screenSize && screenSize.width) || 1;
    const screenHeight = (screenSize && screenSize.height) || 1;
    const containerWidth = resizeEntry.contentRect.width;
    const containerHeight = resizeEntry.contentRect.height;

    const widthFactor = containerWidth / screenWidth;
    const heightFactor = containerHeight / screenHeight;

    // margin top and bottom
    if (widthFactor < heightFactor) {
        const height = screenHeight * widthFactor;
        return {
            left: 0,
            top: (containerHeight - height) / 2,
            width: containerWidth,
            height,
        };
    }

    // margin left and right
    const width = screenWidth * heightFactor;
    return {
        left: (containerWidth - width) / 2,
        top: 0,
        width,
        height: containerHeight,
    };
}
