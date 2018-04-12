import {AfterViewInit, ChangeDetectionStrategy, Component, ElementRef, Input, OnChanges, OnDestroy, OnInit, ViewChild} from '@angular/core';


@Component({
    selector: 'age-emulator',
    template: `
        <div>The Emulator</div>
        <canvas #emulatorCanvas width="160" height="144"></canvas>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeEmulatorComponent implements OnChanges, OnInit, OnDestroy, AfterViewInit {

    @Input()
    emGbModule: EmGbModule;

    @ViewChild('emulatorCanvas')
    private _emulatorCanvas: ElementRef;
    private _canvas2dCtx: CanvasRenderingContext2D | undefined;

    private _timerHandle: number | undefined;
    private _lastEmuTime = 0;


    ngOnInit(): void {
        this._timerHandle = window.setInterval(
            () => this.emulate(),
            10
        );
        this._lastEmuTime = Date.now();
    }

    ngOnDestroy(): void {
        if (this._timerHandle) {
            window.clearInterval(this._timerHandle);
            this._timerHandle = undefined;
        }
    }

    ngAfterViewInit(): void {
        const canvas = this._emulatorCanvas.nativeElement;
        this._canvas2dCtx = canvas.getContext('2d');
    }


    ngOnChanges(/*changes: SimpleChanges*/): void {
        // at the moment ngOnChanges is called only once
        this._lastEmuTime = Date.now();
    }


    private emulate(): void {
        if (this.emGbModule) {

            // calculate the number of elapsed milliseconds since the last emulation call
            const now = Date.now();
            const millisElapsed = now - this._lastEmuTime;
            this._lastEmuTime = now;

            // calculate the number of cycles to emulate based on the elapsed time
            // (limit the cycles to emulate to not freeze the browser in case of performance issues)
            const cyclesPerSecond = this.emGbModule._gb_get_cycles_per_second();
            const maxCyclesToEmulate = cyclesPerSecond / 20;
            const cyclesToEmulate = Math.min(maxCyclesToEmulate, cyclesPerSecond * millisElapsed / 1000);

            // emulate
            const newFrame = this.emGbModule._gb_emulate(cyclesToEmulate);

            // update the canvas if there is a new frame to display
            if (newFrame && this._canvas2dCtx) {
                const screenBuffer = this.emGbModule._gb_get_screen_front_buffer();
                const bytes = new Uint8ClampedArray(this.emGbModule.HEAPU8.buffer, screenBuffer, 160 * 144 * 4);
                const imageData = new ImageData(bytes, 160, 144);
                this._canvas2dCtx.putImageData(imageData, 0, 0);
            }
        }
    }
}
