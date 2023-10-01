(This project is currently unmaintained. It might still work on some Arch Linux but I have observed obvious issues when using a different laptop but don't have time to fix it.)

# modreveal

`modreveal` is a small utility that prints the names of hidden LKMs if any exists.

![Demo](https://github.com/jafarlihi/file-hosting/blob/59be44dca2845a68c210e61ec2733d20ddfad63f/modreveal.gif?raw=true)

## Usage

```
make
sudo ./modreveal
```

## Notes

- To test the utility, you can use the Diamorphine rootkit (https://github.com/m0nad/Diamorphine).
- The author runs Arch Linux LTS kernel, so it is only guaranteed to work on Arch Linux LTS kernel. It will most likely work with your kernel too unless you are running something ancient or really new that breaks something.
