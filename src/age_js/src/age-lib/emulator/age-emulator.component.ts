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
    HostBinding,
    HostListener,
    Input,
    OnDestroy,
    OnInit,
    ViewChild,
} from "@angular/core";
import {AgeResizeObserver, AgeSubscriptionSink} from "../common";
import {AgeEmulationRunner} from "../emulation";
import {AgeEmulator} from "./age-emulator";


@Component({
    selector: "age-emulator",
    template: `
        <canvas #canvas
                [width]="canvasWidth"
                [height]="canvasHeight"
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

        :host.no-emulator canvas {
            display: none;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeEmulatorComponent extends AgeSubscriptionSink implements OnInit, OnDestroy {

    private readonly _emulator = new AgeEmulator();

    @ViewChild("canvas", {static: true}) private _canvas!: ElementRef;
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
    }

    ngOnInit(): void {
        const canvas = this._canvas && this._canvas.nativeElement;
        this._emulator.canvasCtx = canvas.getContext("2d", {alpha: false});

        this.newSubscription = new AgeResizeObserver(
            this._hostElementRef,
            entry => {
                this._hostElementObserverEntry = entry;
                this._calculateViewport();
            },
        );
    }

    async ngOnDestroy() {
        await this._emulator.cleanup();
        super.ngOnDestroy();
    }


    @HostBinding("class.no-emulator") get noEmulator(): boolean {
        return !this._emulator.emulationRunner;
    }

    get canvasStyle(): object {
        return this._canvasStyle;
    }

    get canvasWidth(): number {
        return (this._emulator.emulationRunner && this._emulator.emulationRunner.screenSize.width) || 1;
    }

    get canvasHeight(): number {
        return (this._emulator.emulationRunner && this._emulator.emulationRunner.screenSize.height) || 1;
    }

    @Input() set emulationRunner(emulationRunner: AgeEmulationRunner | undefined) {
        this._emulator.emulationRunner = emulationRunner;
        this._calculateViewport();
    }


    @HostListener("document:keydown", ["$event"]) handleKeyDown(event: KeyboardEvent) {
        this._emulator.handleKeyDown(event);
    }

    @HostListener("document:keyup", ["$event"]) handleKeyUp(event: KeyboardEvent) {
        this._emulator.handleKeyUp(event);
    }


    private _calculateViewport() {
        const contentRect = this._hostElementObserverEntry && this._hostElementObserverEntry.contentRect;
        const emuRunner = this._emulator.emulationRunner;
        if (!contentRect || !emuRunner) {
            return;
        }

        const screenWidth = emuRunner.screenSize.width;
        const screenHeight = emuRunner.screenSize.height;
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
