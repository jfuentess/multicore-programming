import sys
import matplotlib.pyplot as plt
import numpy as np

def fill_matrix(f):
    in_file = open(f)
    
    nrow = int(in_file.readline())
    ncol = int(in_file.readline())
    
    print nrow, ncol
    
    m = np.zeros((nrow,ncol))
    
    x = 0
    y = 0
        
    with in_file as f: 
        for line in f:
            for val in line.split(" "):
                if val == "\n":
                    continue
                m[x,y] = val
                y = y + 1
            x = x + 1
            y = 0

    return m

def main():
    m = fill_matrix(sys.argv[1])
    
    plt.matshow(m)
    plt.show()
            
if __name__ == "__main__":
    main()
