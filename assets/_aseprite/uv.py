from PIL import Image
import os

dir = os.path.dirname(os.path.realpath(__file__))

img = Image.new("RGBA", (256, 256))
pixels = img.load()

def output(uv:int) -> None:
    for i in range(img.size[0]):
        for j in range(img.size[1]):
            pixels[i,j] = (i, j, 0, uv)
    img.save(f"{dir}/uv{uv}.png", "PNG")

output(1)
output(2)
output(3)
output(4)
output(5)
output(6)
output(7)
output(8)
output(9)
output(255)