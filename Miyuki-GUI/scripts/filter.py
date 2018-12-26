import numpy as np
import cv2
import sys
import os   
def denoise(name):
    img = cv2.imread(name)
    dst = cv2.fastNlMeansDenoisingColored(img,None,10,10,7,21)
    cv2.imwrite(name,dst)

if __name__ == "__main__":
    for i in sys.argv[1:]:
        denoise(i)
