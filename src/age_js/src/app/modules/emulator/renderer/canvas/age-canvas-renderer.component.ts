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

import {AfterViewInit, ChangeDetectionStrategy, Component, ElementRef, Input, ViewChild} from '@angular/core';
import {AgeRect} from '../../../common/age-rect';
import {AgeScreenBuffer} from '../../age-emulation';


@Component({
    selector: 'age-canvas-renderer',
    template: `
        <canvas #rendererDisplay
                width="{{screenSize.width}}"
                height="{{screenSize.height}}"
                [style.width]="cssWidth"
                [style.height]="cssHeight"></canvas>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeCanvasRendererComponent implements AfterViewInit {

    @ViewChild('rendererDisplay')
    private _canvas: ElementRef;
    private _canvas2dCtx?: CanvasRenderingContext2D;

    private _screenSize = new AgeRect(1, 1);
    private _cssWidth = '0';
    private _cssHeight = '0';

    ngAfterViewInit(): void {
        const canvas = this._canvas.nativeElement;
        this._canvas2dCtx = canvas.getContext('2d', {alpha: false});
    }


    get cssWidth(): string {
        return this._cssWidth;
    }

    get cssHeight(): string {
        return this._cssHeight;
    }

    get screenSize(): AgeRect {
        return this._screenSize;
    }

    @Input()
    set screenSize(screenSize: AgeRect) {
        this._screenSize = screenSize;
    }

    @Input()
    set viewport(viewport: AgeRect) {
        const widthFactor = viewport.width / this._screenSize.width;
        const heightFactor = viewport.height / this._screenSize.height;

        if (widthFactor < heightFactor) {
            this._cssWidth = `${viewport.width}px`;
            this._cssHeight = `${this._screenSize.height * widthFactor}px`;

        } else {
            this._cssWidth = `${this._screenSize.width * heightFactor}px`;
            this._cssHeight = `${viewport.height}px`;
        }
    }

    @Input()
    set newFrame(screenBuffer: AgeScreenBuffer) {
        if (this._canvas2dCtx) {
            const numBytes = this.screenSize.width * this.screenSize.height * 4;
            const bytes = new Uint8ClampedArray(screenBuffer.buffer.buffer, screenBuffer.offset, numBytes);
            const imageData = new ImageData(bytes, this.screenSize.width, this.screenSize.height);
            this._canvas2dCtx.putImageData(imageData, 0, 0);
        }
    }
}
