import {Directive} from '@angular/core';


@Directive({
    selector: '[ageLoaderWorking]'
})
export class AgeLoaderWorkingDirective {
}


@Directive({
    selector: '[ageLoaderSuccess]'
})
export class AgeLoaderSuccessDirective {
}


@Directive({
    selector: '[ageLoaderError]'
})
export class AgeLoaderErrorDirective {
}
