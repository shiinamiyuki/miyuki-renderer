import cv2
import os
def c(start):
    for thing in os.listdir(start):
        thing = os.path.join(start, thing)
        if os.path.isfile(thing):
            if thing.endswith('.jpg'):
                image = cv2.imread(thing)
                cv2.imwrite(thing[:-4]+'.png', image)
c('./')
