#define GLM_FORCE_RADIANS
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFileDialog>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "model.h"

class GLView : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core 
{
    Q_OBJECT

    public:
        GLView (QWidget *parent=0);
        virtual ~GLView ();
        
    public slots:
    
        void selectFile();
    
        void setProjectionPerspective();
        void setProjectionOrtho();
    
        void centerCamX();
        void centerCamY();
        void centerCamZ();
        
        void enableCamXaxis(bool b);
        void enableCamYaxis(bool b);
        void setCamZaxis(int value);
        
        void setRedLight(int value);
        void setGreenLight(int value);
        void setBlueLight(int value);
        
        void setRedComp(bool b);
        void setGreenComp(bool b);
        void setBlueComp(bool b);
        void setGreyScaleComp(int value);
        
        void setZoom(int value);
        
    signals:
        
        void setZslider(int value);

    protected:
        // initializeGL - Aqui incluim les inicialitzacions del contexte grafic.
        virtual void initializeGL ();
        // paintGL - Mètode cridat cada cop que cal refrescar la finestra.
        // Tot el que es dibuixa es dibuixa aqui.
        virtual void paintGL ();
        // resizeGL - És cridat quan canvia la mida del widget
        virtual void resizeGL (int width, int height);
        // keyPressEvent - Es cridat quan es prem una tecla
        virtual void keyPressEvent (QKeyEvent *event);
        // mouse{Press/Release/Move}Event - Són cridades quan es realitza l'event 
        // corresponent de ratolí
        virtual void mousePressEvent (QMouseEvent *event);
        virtual void mouseReleaseEvent (QMouseEvent *event);
        virtual void mouseMoveEvent (QMouseEvent *event);

    private:
        
        void creaBuffersModel(Model &model, const char *fileName, GLuint *VAO, float &escala, glm::vec3 &centreBase);
        void creaBuffersPatricio();
        void creaBuffersTerra ();
        void carregaShaders ();
        void calculaCapsaModel (Model &m, float &escala, glm::vec3 &centreBase);
        
        void iniEscena ();
        void iniCamera ();
        void projectTransform ();
        void viewTransform ();
        void modelTransformTerra ();
        void modelTransformPatricio ();
        void modelTransformModel ();
        
        void actualitzaColorLlum();
        void actualitzaPosLlum();
        void actualitzaComponents();

        // Program
        QOpenGLShaderProgram *program;
        
        // VAO names
        GLuint VAO_Pat, VAO_Model, VAO_Terra;
        bool recursiveSelectFile;

        // model
        Model patModel;
        Model modModel;
        // paràmetres calculats a partir de la capsa contenidora del model
        glm::vec3 centreBasePat, centreBaseModel;
        float escalaPat, escalaModel;

        // uniform locations
        GLuint transLoc, projLoc, viewLoc, colFocusLoc, llumAmbLoc, posFocusLoc, normalMatLoc, compLoc, greyCoefLoc;

        // escena
        glm::vec3 centreEsc;
        float radiEsc, rav;
        bool xAxisEnabled, yAxisEnabled;
        float angleX, angleY, angleZ;
        
        float redLight, greenLight, blueLight; //Components de color de la llum entre 0 i 1.
        float angleLlumX, angleLlumY, radiLlum;
        
        float fov, fovIni, zn, zf;
        
        float left, right, bottom, top;
        
        bool orthogonal;
        float zoom;
        
        bool redComp, greenComp, blueComp;
        float greyScaleComp;

        // matrius
        glm::mat4 View;

        // attribute locations
        GLuint vertexLoc, normalLoc, matambLoc, matdiffLoc, matspecLoc, matshinLoc;

        GLint ample, alt;

        typedef  enum {NONE, ROTATE, LIGHT} InteractiveAction;
        InteractiveAction DoingInteractive;
        int xClick, yClick;
};

