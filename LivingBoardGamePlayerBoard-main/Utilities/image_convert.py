from PIL import Image
import numpy as np
from math import floor
import argparse


def preprocessing(image):
    return image


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('image_file', type=str)
    args = parser.parse_args()
    image = Image.open(args.image_file)
    image = preprocessing(image)
    image = image.convert(mode='1')
    pages = floor(image.height/8)
    cols = image.width
    file_name = str(args.image_file).replace('.png','')
    with open("../Images/" + file_name.lower()+'.c', 'w') as output_c:
        output_c.writelines([f"#include \"{file_name.lower()}.h\"\n\n", \
                             f"uint8_t {file_name.lower()}[{pages*cols}] = {{"])
        byte = 0
        for i in range(0,pages):
            for j in range(0, cols):
                for k in range(0,8):
                    byte |= (1 if image.getpixel((j, i*8 + k)) > 0 else 0) << k
                output_c.write(str(byte)+', ')
                byte = 0
        output_c.write('};\n')
        
    with open("../Images/" + file_name.lower()+'.h', 'w') as output_h:
        output_h.writelines([f"#ifndef __{file_name.upper()}_H__\n", \
                             f"#define __{file_name.upper()}_H__\n", \
                             f"#include \"pico/stdlib.h\"\n", \
                             f"extern uint8_t {file_name.lower()}[{pages*cols}];\n", \
                             "#endif\n"])
