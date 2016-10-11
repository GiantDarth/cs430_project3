# raycast
### CS430 - Project 2 (Basic Raycaster)
**Christopher Robert Philabaum**

**Northern Arizona University (Fall 2016)**

raycast is a software-renderer-based raycaster that takes in an indefinite amount
of scene objects such as planes and spheres provided by a JSON file, and outputs
the scene as a P6 PPM image file.

**Note:**
* This program chooses to output the PPM file as a P6 raw binary format.

## Usage
`raycast width height /path/to/config.json /path/to/output.ppm`

### parameters:
1. `width`: The width (>0 pixels) of the output image
2. `height`: The height (>0 pixels) of the output image
2. `jsonFile`: A valid path, absolute or relative (to *pwd*), to the config json file.
3. `outputFile`: A valid path, absolute or relative (to *pwd*), to the output ppm file.

All parameters are *required* and not optional. All parameters must be used in the exact order provided above.

## Compile
`make`: Compiles the program into `out/` as `out/raycast`

`make clean`: Removes all object code and the `out/` directory altogether

## Grader Notes
* Because make compiles `raycast` to `out/`, in order to run it properly it should be used as `out/raycast width height /path/to/config.json /path/to/output.ppm`.
* The output file should contain the line/comment "*# Created with raycast (Christopher Philabaum &lt;cp723@nau.edu&gt;)*" following the magic number.
