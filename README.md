# pngtoico

A PNG to ICO converter library that allows you to combine multiple PNG files to generate a single ICO file.  
Try it online: [prashantsinha.com/apps/png-to-ico](https://prashantsinha.com/apps/png-to-ico)

## Installation

```sh
npm install @pkvsinha/pngtoico
```

## Usage

```js
import { convertPngsToIco } from '@pkvsinha/pngtoico';

const handleSubmit = async (e) => {
    e.preventDefault();
    try {
        const files = ["should be file(s) input like e.target.files"];
        const icoBlob = await convertPngToIco(files);
        const url = URL.createObjectURL(icoBlob);
        const link = document.createElement("a");
        link.href = url;
        link.download = "favicon.ico";
        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);
        URL.revokeObjectURL(url);
    } catch (error) {
        console.log(error);
    }
};
```

## Webpack Integration

When using this library with Webpack, you may encounter issues loading the `.wasm` file.  
To resolve this, use the `copy-webpack-plugin` to move the WASM file to your public/assets folder:

```js
const CopyPlugin = require('copy-webpack-plugin');

module.exports = {
  // ...other config
  plugins: [
    new CopyPlugin({
      patterns: [
        {
          from: 'node_modules/@pkvsinha/pngtoico/dist/pngtoico.wasm',
          to: 'pngtoico.wasm'
        }
      ]
    })
  ]
};
```