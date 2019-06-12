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

import {
    AfterViewInit,
    ChangeDetectionStrategy,
    ChangeDetectorRef,
    Component,
    ElementRef,
    Input,
    ViewChild,
} from "@angular/core";
import {AgeResizeObserver, AgeSubscriptionSink} from "../../../common";
import {AgeScreenBuffer} from "../../../emulation";


@Component({
    selector: "age-canvas-renderer",
    template: `
        <canvas #rendererDisplay
                [width]="screenWidth"
                [height]="screenHeight"
                [ngStyle]="canvasStyle"></canvas>
    `,
    styles: [`
        :host {
            /*
             the host element size is used to calculate the canvas element's size
             so it should take up as much space as possible
              */
            display: block;
            height: 100%;
            /* to position the canvas child element, this must be a non-static block */
            position: relative;
        }

        canvas {
            /*
             take the canvas element out of flow,
             so that it does not affect the parent element's size
             => we can calculate the canvas size based on the parent element's size
                without the current canvas size affecting the result
              */
            position: absolute;
            /* center it */
            left: 50%;
            top: 50%;
            transform: translateX(-50%) translateY(-50%);
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeCanvasRendererComponent extends AgeSubscriptionSink implements AfterViewInit {

    @ViewChild("rendererDisplay", {static: false}) private _canvas?: ElementRef;
    private _canvas2dCtx?: CanvasRenderingContext2D;

    private _hostElementObserverEntry?: ResizeObserverEntry;
    private _screenWidth = 1;
    private _screenHeight = 1;
    private _canvasStyle = {
        width: "10px",
        height: "10px",
    };

    constructor(hostElementRef: ElementRef,
                private readonly _changeDetectorRef: ChangeDetectorRef) {
        super();

        this.newSubscription = new AgeResizeObserver(
            hostElementRef,
            entry => {
                this._hostElementObserverEntry = entry;
                this._calculateViewport();
            },
        );
    }

    ngAfterViewInit(): void {
        const canvas = this._canvas && this._canvas.nativeElement;
        this._canvas2dCtx = canvas.getContext("2d", {alpha: false});
    }


    get canvasStyle(): object {
        return this._canvasStyle;
    }

    get screenWidth(): number {
        return this._screenWidth;
    }

    @Input() set screenWidth(screenWidth: number) {
        this._screenWidth = screenWidth;
        this._calculateViewport();
    }

    get screenHeight(): number {
        return this._screenHeight;
    }

    @Input() set screenHeight(screenHeight: number) {
        this._screenHeight = screenHeight;
        this._calculateViewport();
    }

    @Input() set newFrame(screenBuffer: AgeScreenBuffer) {
        if (this._canvas2dCtx) {
            const numBytes = this.screenWidth * this.screenHeight * 4;
            const bytes = new Uint8ClampedArray(screenBuffer.buffer.buffer, screenBuffer.offset, numBytes);
            const imageData = new ImageData(bytes, this.screenWidth, this.screenHeight);
            this._canvas2dCtx.putImageData(imageData, 0, 0);
        }
    }


    private _calculateViewport() {
        if (!this._hostElementObserverEntry) {
            return;
        }

        const viewportWidth = this._hostElementObserverEntry.contentRect.width;
        const viewportHeight = this._hostElementObserverEntry.contentRect.height;

        const widthFactor = viewportWidth / this.screenWidth;
        const heightFactor = viewportHeight / this.screenHeight;

        if (widthFactor < heightFactor) {
            this._canvasStyle = {
                width: `${viewportWidth}px`,
                height: `${this.screenHeight * widthFactor}px`,
            };

        } else {
            this._canvasStyle = {
                width: `${this.screenWidth * heightFactor}px`,
                height: `${viewportHeight}px`,
            };
        }

        this._changeDetectorRef.markForCheck();
    }
}
