import numpy as np


def calcLeftRight(x, y, maxLimit, tol, scale):

    if (abs(x) < tol):
        x = 0
    if (abs(y) < tol):
        y = 0

    #x= (-1)*x
    v = (maxLimit - abs(x))*(y/100)+y
    w = (maxLimit - abs(y))*(x/100)+x

    r = ((v+w)/2)*scale
    l = ((v-w)/2)*scale

    return (r, l, v, w)


if __name__ == '__main__':
    arrX = np.zeros((9,9))
    arrY = np.zeros((9,9))

    teller = 0;
    for i in range(-100, 101, 25):
        arrX[:, teller] = -i
        arrY[teller, :] = -i
        teller+=1

    print(arrX[0][0])
    print(arrY[0][0])
    #arrX[0][0] = 150
    #arrY[0][0] = 160

    v = np.zeros((9,9))
    w = np.zeros((9,9))
    left = np.zeros((9,9))
    right = np.zeros((9,9))

    for i in range(9):
        for j in range(9):
            r1, l1, v1, w1 = calcLeftRight(arrX[i,j], arrY[i,j], 100, 3, 1)
            left[i,j] = l1
            right[i,j] = r1
            v[i,j] = v1
            w[i,j] = w1

    #print(right)
    #print()
    #print(left)
    a, b, c, d = calcLeftRight(-99, 1, 100, 3, 1)
    print(a,b)
