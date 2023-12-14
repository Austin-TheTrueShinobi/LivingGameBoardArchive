from PIL import Image, ImageFilter
import numpy as np
from math import floor
import argparse
import matplotlib.pyplot as plt
from os import listdir
from os.path import isfile, join, getsize
import re


def preprocessing(image):
    image = image.resize((40, 48), Image.LANCZOS)
    return image


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--image_file', '-f', type=str, help='Path to image file to convert')
    parser.add_argument('--c_file_only', '-c', action='store_true', help='Only Generate c and h file of images')
    args = parser.parse_args()
    if not(args.c_file_only):
        image = Image.open(args.image_file)
        if image.width > 40:
            image = preprocessing(image)
        image = image.convert(mode='1')
        pages = floor(image.height/8)
        cols = image.width
        file_name = str(args.image_file).replace('.png','')
        with open("../../Images/" + file_name.lower()+'.c', 'w') as output_c:
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
            
        with open("../../Images/" + file_name.lower()+'.h', 'w') as output_h:
            output_h.writelines([f"#ifndef __{file_name.upper()}_H__\n", \
                                f"#define __{file_name.upper()}_H__\n", \
                                f"#include \"pico/stdlib.h\"\n", \
                                f"extern uint8_t {file_name.lower()}[{pages*cols}];\n", \
                                "#endif\n"])
    images ={'240':[],
             '320':[],
             '24':[]}
    with open("../../Images/images.h", 'w') as image_header:
        image_header.writelines([f"#ifndef __IMAGES_H__\n", \
                                f"#define __IMAGES_H__\n", \
                                "#include \"pico/stdlib.h\"\n"])
        onlyfiles = [f for f in listdir("../../Images") if isfile(join("../../Images", f)) and f.__contains__(".h")]
        for file in onlyfiles:
            with open(join("../../Images", file.replace('.h', '.c')), 'r') as c_file:
                lines = '_'.join(c_file.readlines())
                image_size = lines[lines.find('[')+1:lines.find(']')]
                if image_size in images.keys():
                    images[image_size].append(file.replace('.h','').lower())
            image_header.write(f"#include \"{file}\"\n")
        for key in images.keys():
            image_header.write("extern uint8_t *images_"+key+"[];\n")
        image_header.writelines(["#endif\n"])
    with open("../README.md",'w') as read_me:
        with open("../../Images/images.c", 'w') as image_c:
            image_c.write('#include \"images.h\"\n')
            for key in images.keys():
                if(key == "240"):
                    image_c.write("uint8_t *images_"+key+"[] = { \n NULL,\n")
                else:
                    image_c.write("uint8_t *images_"+key+"[] = {\n")
                for ind,image in enumerate(images[key]):
                    image_c.write("\t"+image+",\n")
                    read_me.write(f"{image}\t\t :  {key}[{ind+1}]\n")
                image_c.write('};\n')