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

const BUFFER_MILLIS = 50;


class Int16Buffer {

    constructor(minBufferSize) {
        this._buffers = [];
        this._bufferOffset = 0;
        this._bufferSize = 0;
        this._minBufferSize = minBufferSize;
        this._buffering = true;
        this._volume = 0.2;
    }

    setVolume(volume) {
        this._volume = Math.min(1, Math.max(0, volume));
    }

    addSamples(buffer) {
        // keep buffered data around the size of this._minBufferSize
        // (don't let the buffer grow too big)
        while (this._bufferSize > this._minBufferSize) {
            this.shiftBuffer();
        }

        // add new buffer
        this._buffers.push(buffer);
        this._bufferSize += buffer.length;

        // continue suspended audio output, if we have enough data now
        this._buffering = this._buffering && (this._bufferSize < this._minBufferSize);
    }

    writeAudioOutput(outputChannels) {
        if (this._buffering) {
            for (let c = 0; c < outputChannels.length; ++c) {
                for (let i = 0; i < outputChannels[c].length; ++i) {
                    outputChannels[c][i] = 0;
                }
            }

        } else {
            for (let outputIdx = 0, outputLength = outputChannels[0].length;
                 this._bufferSize && (outputIdx < outputLength);) {

                const samplesLeft = (this._buffers[0].length - this._bufferOffset) / 2;
                const samples = Math.min(samplesLeft, outputLength - outputIdx);
                const values = samples * 2; // 2 channels

                for (let i = this._bufferOffset, end = i + values;
                     i < end;
                     i += 2, ++outputIdx) {

                    outputChannels[0][outputIdx] = this._volume * this._buffers[0][i] / 32768;
                    outputChannels[1][outputIdx] = this._volume * this._buffers[0][i + 1] / 32768;
                }

                this._bufferOffset += values;
                this._bufferSize -= values;

                const diff = this._buffers[0].length - this._bufferOffset;
                if (diff < 2) {
                    this.shiftBuffer();
                }
            }

            // start buffering, if there are no samples left
            this._buffering = !this._bufferSize;
        }
    }

    shiftBuffer() {
        this._bufferSize -= this._buffers[0].length - this._bufferOffset;
        this._buffers.shift();
        this._bufferOffset = 0;
    }
}


class AgeAudioStream extends AudioWorkletProcessor {

    constructor(options) {
        super(options);
        this.buffer = new Int16Buffer(1);
        this.port.onmessage = this.handleMessage.bind(this);
    }

    handleMessage(event) {
        if (event.data.sampleRate && (event.data.sampleRate !== this.sampleRate)) {
            this.sampleRate = event.data.sampleRate;
            const bufferSize = this.sampleRate * BUFFER_MILLIS * 2 / 1000; // 2 channels
            this.buffer = new Int16Buffer(bufferSize);
        }

        if (event.data.samples) {
            this.buffer.addSamples(event.data.samples);
        }

        if (typeof event.data.volume === 'number') {
            this.buffer.setVolume(event.data.volume);
        }
    }

    /**
     * called once per render quantum (128 samples) according to spec
     */
    process(inputs, outputs) {
        this.buffer.writeAudioOutput(outputs[0]);
        return true;
    }
}

registerProcessor('age-audio-stream', AgeAudioStream);
