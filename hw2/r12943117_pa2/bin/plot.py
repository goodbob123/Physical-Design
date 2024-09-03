import matplotlib.pyplot as plt
import random
import colorsys
import sys


def generate_random_color(saturation_range=(0.9, 1.0)):
    hue = random.random()
    saturation = random.uniform(saturation_range[0], saturation_range[1])
    value = random.random()
    rgb = colorsys.hsv_to_rgb(hue, saturation, value)
    return rgb

def plot_floorplan(blocks):
    fig, ax = plt.subplots()
    for block in blocks:
        x1, y1, x2, y2 = map(int, block.split()[1:])
        width = x2 - x1
        height = y2 - y1
        color = generate_random_color(saturation_range=(0.3, 0.8)) 
        rect = plt.Rectangle((x1, y1), width, height, edgecolor=color, facecolor=color)
        ax.add_patch(rect)
        text_x = x1 + width / 2
        text_y = y1 + height / 2
        ax.text(text_x, text_y, block.split()[0], ha='center', va='center')
        
    ax.autoscale_view()
    name = 'plot_.png'
    plt.savefig(name)

def read_floorplan(file_path):
    with open(file_path, 'r') as f:
        lines = f.readlines()[5:]  # Skip the first 6 lines
        blocks = [line.strip() for line in lines]
    return blocks

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python floorplan_visualizer.py <input_file>")
        sys.exit(1)
        
    file_path = sys.argv[1]
    blocks = read_floorplan(file_path)
    print("Block information:")
    for block in blocks:
        print(block)
    plot_floorplan(blocks)
