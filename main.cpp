#include <iostream>
#include <graphics.h>
#include <math.h>
using namespace std;

double **createArray(int rows, int cols)
{
    double **arr = (double **)malloc(rows * sizeof(double *));
    for (int i = 0; i < rows; i++)
    {
        arr[i] = (double *)malloc(cols * sizeof(double));
    }

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            arr[i][j] = 1.0;
        }
    }
    return arr;
}

struct Part
{
    int type;
    double **vertexMatrix;
    int vertexCount;
    struct Part *nextPart;
    int width;
    int height;
    int id;
    int currentAngle;
    double centerX;
    double centerY;
    int selected;
};

// basic Graphics stuffs
class Graphics2d
{
private:
    double rotationMatrix[3][3];
    double translationMatrix[3][3];
    int offsetVX = -150;
    int offsetVY = 150;

public:
    void multiplyMatrix(double matrixA[3][3], double **matrixB, int cols)
    {
        double matrixC[3][cols];
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                matrixC[i][j] = 0.0;
            }
        }

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                for (int k = 0; k < 3; k++)
                {
                    matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
                }
            }
        }

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                matrixB[i][j] = matrixC[i][j];
            }
        }
    }
    void multiplyMatrix(double matrixA[3][3], double matrixB[3][3])
    {
        double matrixC[3][3];
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                matrixC[i][j] = 0.0;
            }
        }

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                for (int k = 0; k < 3; k++)
                {
                    matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
                }
            }
        }

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                matrixB[i][j] = matrixC[i][j];
            }
        }
    }
    void drawLine(int x0, int y0, int x1, int y1, int color)
    {
        int dy = y1 - y0;
        int dx = x1 - x0;

        int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

        float xInc = dx / (float)steps;
        float yInc = dy / (float)steps;

        float x = x0;
        float y = y0;

        for (int i = 0; i < steps; i++)
        {
            putpixel((int)x + (getmaxx() / 2) + offsetVX, (int)(getmaxy() / 2) - y + offsetVY, color);
            x += xInc;
            y += yInc;
        }
    }
    void drawAxes()
    {
        int originX = (getmaxx() / 2) + offsetVX;
        int originY = (getmaxy() / 2) + offsetVY;
        drawLine(0, 0, originX, 0, COLOR(255, 255, 255));
        drawLine(0, 0, 0, originY, COLOR(255, 255, 255));
        drawLine(0, 0, -1 * originX, 0, COLOR(255, 255, 255));
        drawLine(0, 0, 0, -1 * originY, COLOR(255, 255, 255));
    }
    void drawPart(struct Part *part)
    {
        int color;
        if (part->selected)
        {
            color = COLOR(255, 255, 0);
        }
        else
        {
            color = COLOR(255, 255, 255);
        }
        for (int i = 0; i < part->vertexCount; i++)
        {
            drawLine(part->vertexMatrix[0][i % part->vertexCount], part->vertexMatrix[1][i % part->vertexCount], part->vertexMatrix[0][(i + 1) % part->vertexCount], part->vertexMatrix[1][(i + 1) % part->vertexCount], color);
        }
    }
    void createPolygon(struct Part *part, int vCount, ...)
    {
        va_list args;
        va_start(args, vCount);

        part->vertexMatrix = createArray(3, vCount / 2);
        part->vertexCount = vCount / 2;
        int j = 0;

        for (int i = 0; i < vCount; i += 2)
        {
            int num1 = va_arg(args, int);
            int num2 = va_arg(args, int);
            part->vertexMatrix[0][j] = num1;
            part->vertexMatrix[1][j] = num2;
            j++;
        }
        va_end(args);
    }
    void setTranslateMatrix(double x, double y)
    {
        translationMatrix[0][0] = 1.0;
        translationMatrix[0][1] = 0.0;
        translationMatrix[0][2] = x;
        translationMatrix[1][0] = 0.0;
        translationMatrix[1][1] = 1.0;
        translationMatrix[1][2] = y;
        translationMatrix[2][0] = 0.0;
        translationMatrix[2][1] = 0.0;
        translationMatrix[2][2] = 1.0;
    }
    void setRotateMatrix(double angle)
    {
        double radians = angle * 3.14 / 180.0;
        rotationMatrix[0][0] = cos(radians);
        rotationMatrix[0][1] = -sin(radians);
        rotationMatrix[0][2] = 0.0;
        rotationMatrix[1][0] = sin(radians);
        rotationMatrix[1][1] = cos(radians);
        rotationMatrix[1][2] = 0.0;
        rotationMatrix[2][0] = 0.0;
        rotationMatrix[2][1] = 0.0;
        rotationMatrix[2][2] = 1.0;
    }
    // custom rotate method
    void rotatePart(struct Part *part, double angle, double offsetX, double offsetY)
    {
        setRotateMatrix(angle);
        setTranslateMatrix(offsetX, offsetY);
        multiplyMatrix(translationMatrix, rotationMatrix);
        setTranslateMatrix(-offsetX, -offsetY);
        multiplyMatrix(rotationMatrix, translationMatrix);
        multiplyMatrix(translationMatrix, part->vertexMatrix, part->vertexCount);
    }
};

class Robot
{
private:
    int id = 0;
    struct Part *root;
    struct Part *temp;
    Graphics2d graphics;

public:
    Robot()
    {
        root = createPart(0, 50, 50, COLOR(255, 0, 0));
        graphics.createPolygon(root, 8, 0, 0, 20, 0, 20, 100, 0, 100);

        struct Part *link0 = createPart(0, 50, 50, COLOR(255, 255, 0));
        graphics.createPolygon(link0, 8, 0, 100, 20, 100, 20, 200, 0, 200);
        root->nextPart = link0;

        struct Part *link1 = createPart(0, 50, 50, COLOR(255, 0, 0));
        graphics.createPolygon(link1, 8, 0, 200, 20, 200, 20, 300, 0, 300);
        link0->nextPart = link1;

        struct Part *link2 = createPart(0, 50, 50, COLOR(255, 0, 0));
        graphics.createPolygon(link2, 8, 0, 300, 20, 300, 20, 400, 0, 400);
        link1->nextPart = link2;
    }
    struct Part *createPart(int type, int height, int width, int color)
    {
        struct Part *part;
        part = (struct Part *)malloc(sizeof(struct Part));
        part->type = type;
        part->vertexMatrix = createArray(3, 4);
        part->vertexCount = 4;
        part->height = height;
        part->width = width;
        part->id = this->id++;
        part->currentAngle = 90;
        part->selected = 0;
        return part;
    }
    void Render()
    {
        graphics.drawAxes();
        temp = root;
        while (temp != NULL)
        {
            graphics.drawPart(temp);
            temp->centerX = (temp->vertexMatrix[0][2] + temp->vertexMatrix[0][3]) / 2;
            temp->centerY = (temp->vertexMatrix[1][2] + temp->vertexMatrix[1][3]) / 2;
            temp = temp->nextPart;
        }
    }
    void resetSelection()
    {
        temp = root;
        while (temp != NULL)
        {
            temp->selected = 0;
            temp = temp->nextPart;
        }
    }
    void Rotate(double angle, int idd)
    {
        temp = root;
        double offsetX;
        double offsetY;
        while (temp != NULL)
        {
            if (temp->id == idd)
            {
                temp->selected = 1;
                break;
            }
            offsetX = temp->centerX;
            offsetY = temp->centerY;
            temp = temp->nextPart;
        }
        if (temp == NULL)
        {
            cout << "CANT FIND";
            return;
        }
        temp->currentAngle += angle;
        if (temp->currentAngle >= 180)
        {
            temp->currentAngle -= angle;
            return;
        }
        if (temp->currentAngle <= 0)
        {
            temp->currentAngle -= angle;
            return;
        }

        while (temp != NULL)
        {
            graphics.rotatePart(temp, angle, offsetX, offsetY);
            temp = temp->nextPart;
        }
    }
};

int main()
{
    int gd = DETECT;
    int gm;
    initgraph(&gd, &gm, (char *)"");
    char choice;
    Robot robot;
    int page = 1;
    // int left = 0;
    // int top = 0;
    // int right = getmaxx();
    // int bottom = getmaxy();
    int currentId = 2;
    int rotationStep = 5;
    robot.Render();
    while (1)
    {
        setactivepage(page);
        setvisualpage(1 - page);
        cleardevice();
        choice = getch();
        switch (choice)
        {
        case 'K':
        {
            robot.Rotate(rotationStep, currentId);
            break;
        }
        case 'M':
        {
            robot.Rotate(-rotationStep, currentId);
            break;
        }
        case 'H':
        {
            if (currentId + 1 < 4)
            {
                currentId++;
                robot.resetSelection();
            }
            break;
        }
        case 'P':
        {
            if (currentId - 1 > 0)
            {
                currentId--;
                robot.resetSelection();
            }
            break;
        }
        default:
            break;
        }
        robot.Render();
        page = 1 - page;
        cout << choice;
    }
    closegraph();
    return 0;
}