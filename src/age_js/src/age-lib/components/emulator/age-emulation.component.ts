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
    ChangeDetectionStrategy,
    ChangeDetectorRef,
    Component,
    ElementRef,
    HostListener,
    Input,
    OnInit,
    ViewChild,
} from "@angular/core";
import {AgeResizeObserver, AgeSubscriptionSink} from "../../common";
import {AgeEmulation, AgeScreenSize} from "../../emulation";
import {AgeEmulationWorker} from "./age-emulation-worker";


@Component({
    selector: "age-emulation",
    template: `
        <canvas #canvas
                [width]="screenSize.width"
                [height]="screenSize.height"
                [ngStyle]="canvasStyle"></canvas>
    `,
    styles: [`
        :host {
            /*
             The host element size is used to calculate the canvas element's size.
             It's up to the former's parent element to set a suitable size.
              */
            display: block;
            /* to position the canvas child element, this must be a non-static block */
            position: relative;
            overflow: hidden;
        }

        canvas {
            /*
             take the canvas element out of flow,
             so that it does not affect the host element's size
             => we can calculate the canvas size based on the host element's size
                without the current canvas size affecting the result
              */
            position: absolute;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeEmulationComponent extends AgeSubscriptionSink implements OnInit {

    private readonly _emulationWorker = new AgeEmulationWorker();

    @ViewChild("canvas", {static: true}) private _canvas?: ElementRef;
    private _canvasStyle = {
        left: "0px",
        top: "0px",
        width: "10px",
        height: "10px",
    };
    private _hostElementObserverEntry?: ResizeObserverEntry;


    constructor(private readonly _hostElementRef: ElementRef,
                private readonly _changeDetectorRef: ChangeDetectorRef) {
        super();
        this.newSubscription = this._emulationWorker;
    }

    ngOnInit(): void {
        const canvas: HTMLCanvasElement | undefined = this._canvas && this._canvas.nativeElement;
        this._emulationWorker.canvasCtx = canvas && canvas.getContext("2d", {alpha: false});

        this.newSubscription = new AgeResizeObserver(
            this._hostElementRef,
            entry => {
                this._hostElementObserverEntry = entry;
                this._calculateViewport();
            },
        );
    }


    get canvasStyle(): object {
        return this._canvasStyle;
    }

    get screenSize(): AgeScreenSize {
        return (this._emulationWorker.emulation && this._emulationWorker.emulation.screenSize)
            || {width: 1, height: 1};
    }

    @Input() set emulation(emulation: AgeEmulation) {
        this._emulationWorker.emulation = emulation;
        this._calculateViewport();
    }

    @Input() set pauseEmulation(pauseEmulation: boolean) {
        this._emulationWorker.pauseEmulation = pauseEmulation;
    }


    @HostListener("document:keydown", ["$event"]) handleKeyDown(event: KeyboardEvent) {
        this._emulationWorker.handleKeyDown(event);
    }

    @HostListener("document:keyup", ["$event"]) handleKeyUp(event: KeyboardEvent) {
        this._emulationWorker.handleKeyUp(event);
    }


    private _calculateViewport() {
        const contentRect = this._hostElementObserverEntry && this._hostElementObserverEntry.contentRect;
        if (!contentRect) {
            return;
        }

        const screenWidth = this.screenSize.width;
        const screenHeight = this.screenSize.height;
        const viewportWidth = contentRect.width;
        const viewportHeight = contentRect.height;

        const widthFactor = viewportWidth / screenWidth;
        const heightFactor = viewportHeight / screenHeight;

        if (widthFactor < heightFactor) {
            // margin top and bottom
            const height = screenHeight * widthFactor;
            this._canvasStyle = {
                left: "0px",
                top: `${(viewportHeight - height) / 2}px`,
                width: `${viewportWidth}px`,
                height: `${height}px`,
            };

        } else {
            // margin left and right
            const width = screenWidth * heightFactor;
            this._canvasStyle = {
                left: `${(viewportWidth - width) / 2}px`,
                top: "0px",
                width: `${width}px`,
                height: `${viewportHeight}px`,
            };
        }

        this._changeDetectorRef.markForCheck();
    }
}
