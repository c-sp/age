//
// Copyright 2019 Christoph Sprenger
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

import {ChangeDetectionStrategy, Component, ElementRef, HostListener, Input, OnInit, ViewChild} from '@angular/core';
import {AgeSubscriptionSink} from '../../common';
import {AgeEmulation, AgeScreenSize} from '../../emulation';
import {AgeEmulationWorker} from './age-emulation-worker';


@Component({
    selector: 'age-emulation',
    template: `
        <canvas #canvas
                [width]="screenSize.width"
                [height]="screenSize.height"></canvas>
    `,
    styles: [`
        :host {
            /* prevent flickering scrollbar on rapid resize */
            overflow: hidden;
        }

        canvas {
            /* we expect the parent element to be sized accordingly */
            width: 100%;
            height: 100%;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeEmulationComponent extends AgeSubscriptionSink implements OnInit {

    private readonly _emulationWorker = new AgeEmulationWorker();
    @ViewChild('canvas', {static: true}) private _canvas?: ElementRef;

    constructor() {
        super();
        this.newSubscription = this._emulationWorker;
    }

    ngOnInit(): void {
        const canvas: HTMLCanvasElement | undefined = this._canvas && this._canvas.nativeElement;
        this._emulationWorker.canvasCtx = canvas && canvas.getContext('2d', {alpha: false});
    }


    get screenSize(): AgeScreenSize {
        return (this._emulationWorker.emulation && this._emulationWorker.emulation.screenSize)
            || {width: 1, height: 1};
    }

    @Input() set audioVolume(volume: number) {
        this._emulationWorker.audioVolume = volume;
    }

    @Input() set emulation(emulation: AgeEmulation) {
        this._emulationWorker.emulation = emulation;
    }

    @Input() set pauseEmulation(pauseEmulation: boolean) {
        this._emulationWorker.pauseEmulation = pauseEmulation;
    }


    @HostListener('document:keydown', ['$event']) handleKeyDown(event: KeyboardEvent) {
        this._emulationWorker.handleKeyDown(event);
    }

    @HostListener('document:keyup', ['$event']) handleKeyUp(event: KeyboardEvent) {
        this._emulationWorker.handleKeyUp(event);
    }
}
