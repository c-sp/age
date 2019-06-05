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
    ElementRef, HostBinding,
    Input,
    OnDestroy,
    ViewChild,
} from "@angular/core";
import ResizeObserver from "resize-observer-polyfill";
import {AgeRect} from "../../../common";
import {AgeScreenBuffer} from "../../age-emulation";


@Component({
    selector: "age-canvas-renderer",
    template: `
        <canvas #rendererDisplay
                width="{{screenSize.width}}"
                height="{{screenSize.height}}"
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
        }

        canvas {
            /*
             take the canvas element out of flow,
             so that it does not affect the parent element's size
              */
            position: absolute;
            /* center it horizontally */
            left: 50%;
            transform: translateX(-50%);
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeCanvasRendererComponent implements AfterViewInit, OnDestroy {

    private readonly _resizeObserver: ResizeObserver;

    @ViewChild("rendererDisplay", {static: false}) private _canvas?: ElementRef;
    private _canvas2dCtx?: CanvasRenderingContext2D;

    private _hostElementSize = new AgeRect(1, 1);
    private _screenSize = new AgeRect(1, 1);
    private _canvasStyle = {
        width: "10px",
        height: "10px",
    };
    private _hostMinWidth = "10px";
    private _hostMinHeight = "10px";

    constructor(hostElementRef: ElementRef,
                private readonly _changeDetectorRef: ChangeDetectorRef) {

        this._resizeObserver = new ResizeObserver(entries => {
            // we observe a single element and thus use only the first entry
            const entry = entries && entries.length && entries[0];
            if (entry) {
                this._hostElementSize = new AgeRect(
                    entry.contentRect.width,
                    entry.contentRect.height,
                );
                this._calculateViewport();
            }
        });
        this._resizeObserver.observe(hostElementRef.nativeElement);
    }

    ngAfterViewInit(): void {
        const canvas = this._canvas && this._canvas.nativeElement;
        this._canvas2dCtx = canvas.getContext("2d", {alpha: false});
    }

    ngOnDestroy(): void {
        this._resizeObserver.disconnect();
    }


    get canvasStyle(): object {
        return this._canvasStyle;
    }

    @HostBinding("style.minWidth")
    get hostMinWidth(): string {
        return this._hostMinWidth;
    }

    @HostBinding("style.minHeight")
    get hostMinHeight(): string {
        return this._hostMinHeight;
    }

    get screenSize(): AgeRect {
        return this._screenSize;
    }

    @Input() set screenSize(screenSize: AgeRect) {
        this._screenSize = screenSize;
        this._hostMinWidth = `${screenSize.width}px`;
        this._hostMinHeight = `${screenSize.height}px`;
        this._calculateViewport();
    }

    @Input() set newFrame(screenBuffer: AgeScreenBuffer) {
        if (this._canvas2dCtx) {
            const numBytes = this.screenSize.width * this.screenSize.height * 4;
            const bytes = new Uint8ClampedArray(screenBuffer.buffer.buffer, screenBuffer.offset, numBytes);
            const imageData = new ImageData(bytes, this.screenSize.width, this.screenSize.height);
            this._canvas2dCtx.putImageData(imageData, 0, 0);
        }
    }


    private _calculateViewport() {
        const viewportWidth = this._hostElementSize.width;
        const viewportHeight = this._hostElementSize.height;

        const widthFactor = viewportWidth / this._screenSize.width;
        const heightFactor = viewportHeight / this._screenSize.height;

        if (widthFactor < heightFactor) {
            this._canvasStyle = {
                width: `${viewportWidth}px`,
                height: `${Math.floor(this._screenSize.height * widthFactor)}px`,
            };

        } else {
            this._canvasStyle = {
                width: `${Math.floor(this._screenSize.width * heightFactor)}px`,
                height: `${viewportHeight}px`,
            };
        }

        this._changeDetectorRef.markForCheck();
    }
}
