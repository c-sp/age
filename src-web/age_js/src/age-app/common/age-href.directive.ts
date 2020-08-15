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

import {Directive, ElementRef, Input, OnInit, Renderer2} from '@angular/core';


@Directive({
    selector: 'a[age-href]',
})
export class AgeHrefDirective implements OnInit {

    // tslint:disable-next-line:no-input-rename
    @Input('age-href') ageHref?: string;

    constructor(private readonly _renderer: Renderer2,
                private readonly _elementRef: ElementRef) {
    }

    ngOnInit(): void {
        this._renderer.setProperty(this._elementRef.nativeElement, 'href', this.ageHref || '');
        this._renderer.setProperty(this._elementRef.nativeElement, 'target', '_blank');
        this._renderer.setProperty(this._elementRef.nativeElement, 'rel', 'noopener nofollow noreferrer');
    }
}
