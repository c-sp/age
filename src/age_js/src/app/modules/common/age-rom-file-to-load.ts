export class AgeRomFileToLoad {

    static forLocalFile(file: File) {
        return new AgeRomFileToLoad(file);
    }

    static forUrl(url: string) {
        if (!url) { // prevent empty strings
            throw new Error('url not specified');
        }
        return new AgeRomFileToLoad(undefined, url);
    }

    private constructor(readonly file?: File,
                        readonly url?: string) {
    }
}

