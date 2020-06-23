#include "GLView.h"

#include <iostream>

GLView::GLView (QWidget* parent) : QOpenGLWidget(parent), program(NULL)
{
    setFocusPolicy(Qt::StrongFocus);  // per rebre events de teclat
    xClick = yClick = 0;
    DoingInteractive = NONE;
    recursiveSelectFile = false;
}

GLView::~GLView ()
{
    if (program != NULL)
        delete program;
}

void GLView::initializeGL ()
{
    // Cal inicialitzar l'ús de les funcions d'OpenGL
    initializeOpenGLFunctions();  

    glClearColor(0.7, 1.0, 1.0, 1.0); // defineix color de fons (d'esborrat)
    glEnable(GL_DEPTH_TEST);
    carregaShaders();
    iniEscena ();
    iniCamera ();
}

void GLView::iniEscena ()
{
    creaBuffersTerra();
    creaBuffersFocus();
    creaBuffersModel(modModel, "../models/Patricio.obj", &VAO_Model, escalaModel, centreBaseModel, midesModel);

    centreEsc = glm::vec3 (0, 2, 0);
    radiEsc = sqrt(66+4)/2;
    
    redLight = greenLight = blueLight = 1.f;
    glm::vec3 cF(1, 1, 1);
    glUniform3fv (colFocusLoc, 1, &cF[0]);
    
    glm::vec3 lA(0.2, 0.2, 0.2);
    glUniform3fv (llumAmbLoc, 1, &lA[0]);
    
    angleLlumX = angleLlumY = 0.0f;
    radiLlum = radiEsc*1.5f;
    actualitzaPosLlum();
    
    redComp = greenComp = blueComp = true;
    greyScaleComp = 0.f;
    actualitzaComponents();
    
    showFloor = true;
}

void GLView::iniCamera ()
{
    rav = 1.0;
    xAxisEnabled = yAxisEnabled = true;
    angleX = angleY = angleZ = 0.0;
    
    fovIni = fov = float(M_PI/3.0);
    zn = 2*radiEsc-0.92*radiLlum;
    zf = 2*radiEsc+1.2*radiLlum;
    
    left = -radiEsc;
    right = radiEsc;
    bottom = -radiEsc;
    top = radiEsc;
    
    orthogonal = false;
    zoom = 1;
    
    projectTransform ();
    viewTransform ();
}

void GLView::paintGL () 
{
    // Aquest codi és necessari únicament per a MACs amb pantalla retina.
    #ifdef __APPLE__
    GLint vp[4];
    glGetIntegerv (GL_VIEWPORT, vp);
    ample = vp[2];
    alt = vp[3];
    #endif

    // En cas de voler canviar els paràmetres del viewport, descomenteu la crida següent i
    // useu els paràmetres que considereu (els que hi ha són els de per defecte)
    //  glViewport (0, 0, ample, alt);
    
    // Esborrem el frame-buffer i el depth-buffer
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (showFloor) {
        //--------------------------------------------------------
        // Activem el VAO per a pintar el terra
        glBindVertexArray (VAO_Terra);  
        // pintem terra
        modelTransformTerra ();
        glDrawArrays(GL_TRIANGLES, 0, 12);
    }
    

    //--------------------------------------------------------
    // Activem el VAO per a pintar el Focus
    glBindVertexArray (VAO_Focus);
    // pintem el Focus
    modelTransformFocus();
    glDrawArrays(GL_TRIANGLES, 0, focusModel.faces().size()*3);
    
    // Activem el VAO per a pintar el Model
    glBindVertexArray (VAO_Model);
    // pintem el Model
    modelTransformModel();
    glDrawArrays(GL_TRIANGLES, 0, modModel.faces().size()*3);
    

    //--------------------------------------------------------
    glBindVertexArray(0);
}

void GLView::resizeGL (int w, int h)
{
    ample = w;
    alt = h;
    rav =  float(ample) / float(alt);
    
    if (rav < 1) {
        fov = 2*atan(tan(fovIni/2)/rav);
        left = -radiEsc;
        right = radiEsc;
        bottom = -radiEsc/rav;
        top = radiEsc/rav;
    } else if (rav > 1) {
        left = -radiEsc*rav;
        right = radiEsc*rav;
        bottom = -radiEsc;
        top = radiEsc;
    }
    
    projectTransform ();
}

void GLView::modelTransformTerra ()
{
    glm::mat4 TG(1.f);  // Matriu de transformació
    glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);    
    
    glm::mat3 Normal = glm::inverse(glm::transpose(glm::mat3(View*TG)));
    glUniformMatrix3fv (normalMatLoc, 1, GL_FALSE, &Normal[0][0]);
}

void GLView::modelTransformFocus ()
{
    // Codi per a la TG necessària
    glm::mat4 focusTG = glm::inverse(View)*llumTG;
    focusTG = glm::translate(focusTG, glm::vec3(0,0,radiLlum*.03f));
    focusTG = glm::rotate(focusTG, float(-M_PI)/2, glm::vec3(0, 0, 1));
    focusTG = glm::rotate(focusTG, float(-M_PI)/2, glm::vec3(0, 1, 0));
    focusTG = glm::translate(focusTG, glm::vec3(0, -midesFocus.y/2, 0));
    focusTG = glm::scale(focusTG, glm::vec3(escalaFocus, escalaFocus, escalaFocus));
    focusTG = glm::translate(focusTG, -centreBaseFocus);
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &focusTG[0][0]);
    
    glm::mat3 Normal = glm::inverse(glm::transpose(glm::mat3(View*focusTG)));
    glUniformMatrix3fv (normalMatLoc, 1, GL_FALSE, &Normal[0][0]);
}

void GLView::modelTransformModel ()
{
    // Codi per a la TG necessària
    glm::mat4 modelTG = glm::mat4(1.0f);
    modelTG = glm::scale(modelTG, glm::vec3(4, 4, 4));
    modelTG = glm::scale(modelTG, glm::vec3(escalaModel, escalaModel, escalaModel));
    modelTG = glm::translate(modelTG, -centreBaseModel);
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, &modelTG[0][0]);
    
    glm::mat3 Normal = glm::inverse(glm::transpose(glm::mat3(View*modelTG)));
    glUniformMatrix3fv (normalMatLoc, 1, GL_FALSE, &Normal[0][0]);
}

void GLView::projectTransform ()
{
    glm::mat4 Proj;  // Matriu de projecció

    if (orthogonal) {
        if (zoom > 1) {
            Proj = glm::ortho(left*zoom*zoom, right*zoom*zoom, bottom*zoom*zoom, top*zoom*zoom, zn, zf);
        } else {
            Proj = glm::ortho(left*zoom, right*zoom, bottom*zoom, top*zoom, zn, zf);
        }
    } else {
        Proj = glm::perspective(fov*zoom, rav, zn, zf);
    }

    glUniformMatrix4fv (projLoc, 1, GL_FALSE, &Proj[0][0]);
}

void GLView::viewTransform ()
{
    // Matriu de posició i orientació
    View = glm::translate(glm::mat4(1.f), glm::vec3(0, 0, -2*radiEsc));
    View = glm::rotate(View, angleZ, glm::vec3(0, 0, 1));
    View = glm::rotate(View, -angleX, glm::vec3(1, 0, 0));
    View = glm::rotate(View, angleY, glm::vec3(0, 1, 0));
    View = glm::translate(View, -centreEsc);

    glUniformMatrix4fv (viewLoc, 1, GL_FALSE, &View[0][0]);
}

void GLView::selectFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open .obj file", "../models", "OBJ (*.obj)");
    if (fileName == "") return;
    
    makeCurrent();
    
    creaBuffersModel(modModel, fileName.toLatin1().constData(), &VAO_Model, escalaModel, centreBaseModel, midesModel);
    centerCamZ();
    
    update();
}

void GLView::setProjectionPerspective() {
    makeCurrent();
    orthogonal = false;
    projectTransform();
    update();
}

void GLView::setProjectionOrtho() {
    makeCurrent();
    orthogonal = true;
    projectTransform();
    update();
}

void GLView::centerCamX() {
    makeCurrent();
    
    angleX = 0;
    angleY = -M_PI/2;
    angleZ = 0;
    emit setZslider(0);
    viewTransform();
    
    update();
}

void GLView::centerCamY() {
    makeCurrent();
    
    angleX = -M_PI/2;
    angleY = 0;
    angleZ = 0;
    emit setZslider(0);
    viewTransform();
    
    update();
}

void GLView::centerCamZ() {
    makeCurrent();
    
    angleX = 0;
    angleY = 0;
    angleZ = 0;
    emit setZslider(0);
    viewTransform();
    
    update();
}

void GLView::enableCamXaxis(bool b) {
    makeCurrent();
    xAxisEnabled = b;
    update();
}

void GLView::enableCamYaxis(bool b) {
    makeCurrent();
    yAxisEnabled = b;
    update();
}

void GLView::setCamZaxis(int value) {
    makeCurrent();
    angleZ = float(value)*M_PI/180;
    viewTransform();
    update();
}

//pre: value [0..100]
void GLView::setRedLight(int value) {
    makeCurrent();
    
    redLight = float(value)/100;
    actualitzaColorLlum();
    
    update();
}

//pre: value [0..100]
void GLView::setGreenLight(int value) {
    makeCurrent();
    
    greenLight = float(value)/100;
    actualitzaColorLlum();
    
    update();
}

//pre: value [0..100]
void GLView::setBlueLight(int value) {
    makeCurrent();
    
    blueLight = float(value)/100;
    actualitzaColorLlum();
    
    update();
}

void GLView::actualitzaColorLlum() {
    glm::vec3 cF(redLight, greenLight, blueLight);
    glUniform3fv (colFocusLoc, 1, &cF[0]);
}

void GLView::actualitzaPosLlum() {
    llumTG = glm::translate(glm::mat4(1.f), glm::vec3(0, 0, -2*radiEsc));
    llumTG = glm::rotate(llumTG, angleLlumY, glm::vec3(0, 1, 0));
    llumTG = glm::rotate(llumTG, -angleLlumX, glm::vec3(1, 0, 0));
    llumTG = glm::translate(llumTG, glm::vec3(0,0,radiLlum));
    
    glm::vec4 llum(0,0,0,1);
    llum = llumTG*llum;
    
    glUniform4fv (posFocusLoc, 1, &llum[0]);
}

void GLView::setDarkBackground() {
    makeCurrent();
    glClearColor(0.0,0.1,0.1,1.0);
    update();
}

void GLView::setLightBackground() {
    makeCurrent();
    glClearColor(0.7,1.0,1.0,1.0);
    update();
}

void GLView::setShowFloor(bool b) {
    makeCurrent();
    showFloor = b;
    update();
}

void GLView::setRedComp(bool b) {
    makeCurrent();
    
    redComp = b;
    actualitzaComponents();
    
    update();
}

void GLView::setGreenComp(bool b) {
    makeCurrent();
    
    greenComp = b;
    actualitzaComponents();
    
    update();
}

void GLView::setBlueComp(bool b) {
    makeCurrent();
    
    blueComp = b;
    actualitzaComponents();
    
    update();
}

void GLView::setGreyScaleComp(int value) {
    makeCurrent();
    
    greyScaleComp = float(value)/100;
    actualitzaComponents();
    
    update();
}

void GLView::actualitzaComponents() {
    glm::ivec4 comp(redComp,greenComp,blueComp,greyScaleComp!=0);
    glUniform4iv (compLoc, 1, &comp[0]);
    
    glUniform1f(greyCoefLoc, greyScaleComp);
}

void GLView::setZoom(int value) {
    makeCurrent();
    
    zoom = float(value)/10;
    if (zoom < 0.2f) zoom = 0.2f;
    else if (zoom > 2.3f) zoom = 2.3f;
    projectTransform();
    
    update();
}

void GLView::keyPressEvent(QKeyEvent* event)  // Cal modificar aquesta funció...
{
    makeCurrent();
    switch (event->key()) {
        case Qt::Key_P: {
            orthogonal = !orthogonal;
            projectTransform();
            if (orthogonal) {
                emit toggleOrtho();
            } else {
                emit togglePerspective();
            }
            break;
        }
        case Qt::Key_L: {
            setLightBackground();
            emit toggleLightBack();
            break;
        }
        case Qt::Key_D: {
            setDarkBackground();
            emit toggleDarkBack();
            break;
        }
        case Qt::Key_X: {
            centerCamX();
            break;
        }
        case Qt::Key_Y: {
            centerCamY();
            break;
        }
        case Qt::Key_Z: {
            centerCamZ();
            break;
        }
        case Qt::Key_F: {
            showFloor = !showFloor;
            emit signalShowFloor(showFloor);
            break;
        }
        case Qt::Key_R: {
            redComp = !redComp;
            actualitzaComponents();
            emit signalRedComp(redComp);
            break;
        }
        case Qt::Key_G: {
            greenComp = !greenComp;
            actualitzaComponents();
            emit signalGreenComp(greenComp);
            break;
        }
        case Qt::Key_B: {
            blueComp = !blueComp;
            actualitzaComponents();
            emit signalBlueComp(blueComp);
            break;
        }
        default: event->ignore(); break;
    }

    update();
}

void GLView::mousePressEvent (QMouseEvent *e)
{
    xClick = e->x();
    yClick = e->y();

    if (!(e->modifiers() & (Qt::ShiftModifier|Qt::AltModifier|Qt::ControlModifier)))
    {
        if (e->button() & Qt::LeftButton) DoingInteractive = ROTATE;
        else if (e->button() & Qt::RightButton) DoingInteractive = LIGHT;
    }
}

void GLView::mouseReleaseEvent( QMouseEvent *)
{
    DoingInteractive = NONE;
}

void GLView::mouseMoveEvent(QMouseEvent *e)
{
    makeCurrent();
    if (DoingInteractive == ROTATE) {
        // Fem la rotació
        if (yAxisEnabled) angleY += (e->x() - xClick) * M_PI / 180.0;    
        if (xAxisEnabled) angleX += (yClick - e->y()) * M_PI / 180.0;    
        if (xAxisEnabled or yAxisEnabled) viewTransform ();
    } else if (DoingInteractive == LIGHT) {
        // Fem la rotació de la llum
        angleLlumY += (e->x() - xClick) * M_PI / 270.0;    
        angleLlumX += (yClick - e->y()) * M_PI / 270.0;    
        actualitzaPosLlum ();
    }

    xClick = e->x();
    yClick = e->y();

    update ();
}

void GLView::calculaCapsaModel (Model &m, float &escala, glm::vec3 &centreBase, glm::vec3 &scaledSizes)
{
    // Càlcul capsa contenidora i valors transformacions inicials
    float minx, miny, minz, maxx, maxy, maxz;
    minx = maxx = m.vertices()[0];
    miny = maxy = m.vertices()[1];
    minz = maxz = m.vertices()[2];
    for (unsigned int i = 3; i < m.vertices().size(); i+=3)
    {
        if (m.vertices()[i+0] < minx)
        minx = m.vertices()[i+0];
        if (m.vertices()[i+0] > maxx)
        maxx = m.vertices()[i+0];
        if (m.vertices()[i+1] < miny)
        miny = m.vertices()[i+1];
        if (m.vertices()[i+1] > maxy)
        maxy = m.vertices()[i+1];
        if (m.vertices()[i+2] < minz)
        minz = m.vertices()[i+2];
        if (m.vertices()[i+2] > maxz)
        maxz = m.vertices()[i+2];
    }
    
    if (maxx-minx > maxy-miny && maxx-minx > maxz-minz) {
        escala = 1.0/(maxx-minx);
    } else if (maxy-miny > maxz-minz) {
        escala = 1.0/(maxy-miny);
    } else {
        escala = 1.0/(maxz-minz);
    }
    
    centreBase[0] = (minx+maxx)/2.0;
    centreBase[1] = miny;
    centreBase[2] = (minz+maxz)/2.0;
    
    scaledSizes[0] = (maxx-minx)*escala;
    scaledSizes[1] = (maxy-miny)*escala;
    scaledSizes[2] = (maxz-minz)*escala;
}

void GLView::creaBuffersFocus(){
    creaBuffersModel(focusModel, "../models/focus.obj", &VAO_Focus, escalaFocus, centreBaseFocus, midesFocus);
}

void GLView::creaBuffersModel(Model &model, const char *fileName,
				  GLuint *VAO,  float &escala,
				  glm::vec3 &centreBase, glm::vec3 &scaledSizes)
{
    // Carreguem el model de l'OBJ - Atenció! Abans de crear els buffers!
    model.load(fileName);
    
    std::cout<<"creantBufferModel  "<<fileName<<std::endl;

    // Calculem la capsa contenidora del model
    calculaCapsaModel(model, escala, centreBase, scaledSizes);
    
    // Creació del Vertex Array Object del model
    glGenVertexArrays(1, VAO);
    glBindVertexArray(*VAO);

    // Creació dels buffers del model
    GLuint VBO_Model[6];
    // Buffer de posicions
    glGenBuffers(6, VBO_Model);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Model[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*model.faces().size()*3*3, model.VBO_vertices(), GL_STATIC_DRAW);

    // Activem l'atribut vertexLoc
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vertexLoc);

    // Buffer de normals
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Model[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*model.faces().size()*3*3, model.VBO_normals(), GL_STATIC_DRAW);

    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(normalLoc);

    // Buffer de component ambient
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Model[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*model.faces().size()*3*3, model.VBO_matamb(), GL_STATIC_DRAW);

    glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matambLoc);

    // Buffer de component difusa
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Model[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*model.faces().size()*3*3, model.VBO_matdiff(), GL_STATIC_DRAW);

    glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matdiffLoc);

    // Buffer de component especular
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Model[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*model.faces().size()*3*3, model.VBO_matspec(), GL_STATIC_DRAW);

    glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matspecLoc);

    // Buffer de component shininness
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Model[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*model.faces().size()*3, model.VBO_matshin(), GL_STATIC_DRAW);

    glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matshinLoc);

    glBindVertexArray(0);
}

void GLView::creaBuffersTerra ()
{
    // VBO amb la posició dels vèrtexs
    glm::vec3 posterra[12] = {
        glm::vec3(-2.5, 0.0, 2.5),
        glm::vec3(2.5, 0.0, 2.5),
        glm::vec3(-2.5, 0.0, -2.5),
        glm::vec3(-2.5, 0.0, -2.5),
        glm::vec3(2.5, 0.0, 2.5),
        glm::vec3(2.5, 0.0, -2.5),
        glm::vec3(-2.5, -0.001, 2.5),
        glm::vec3(2.5, -0.001, 2.5),
        glm::vec3(-2.5, -0.001, -2.5),
        glm::vec3(-2.5, -0.001, -2.5),
        glm::vec3(2.5, -0.001, 2.5),
        glm::vec3(2.5, -0.001, -2.5)
    }; 

    // VBO amb la normal de cada vèrtex
    glm::vec3 norm1 (0,1,0);
    glm::vec3 norm2 (0,-1,0);
    glm::vec3 normterra[12] = {
        norm1, norm1, norm1, norm1, norm1, norm1, // la normal (0,1,0) per als dos triangles de sobre
        norm2, norm2, norm2, norm2, norm2, norm2 // la normal (0,1,0) per als dos triangles de sota
    };

    // Definim el material del terra
    glm::vec3 amb(0.000000, 0.050000 ,0.200000);
    glm::vec3 diff(0.000000, 0.200000 ,0.800000);
    glm::vec3 spec(0.2,0.2,0.2);
    float shin = 100;

    // Fem que aquest material afecti a tots els vèrtexs per igual
    glm::vec3 matamb[12] = {
        amb, amb, amb, amb, amb, amb, 
        amb, amb, amb, amb, amb, amb
    };
    glm::vec3 matdiff[12] = {
        diff, diff, diff, diff, diff, diff, 
        diff, diff, diff, diff, diff, diff
    };
    glm::vec3 matspec[12] = {
        spec, spec, spec, spec, spec, spec, 
        spec, spec, spec, spec, spec, spec
    };
    float matshin[12] = {
        shin, shin, shin, shin, shin, shin, 
        shin, shin, shin, shin, shin, shin
    };

    // Creació del Vertex Array Object del terra
    glGenVertexArrays(1, &VAO_Terra);
    glBindVertexArray(VAO_Terra);

    GLuint VBO_Terra[6];
    glGenBuffers(6, VBO_Terra);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(posterra), posterra, GL_STATIC_DRAW);

    // Activem l'atribut vertexLoc
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vertexLoc);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normterra), normterra, GL_STATIC_DRAW);

    // Activem l'atribut normalLoc
    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(normalLoc);

    // Buffer de component ambient
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(matamb), matamb, GL_STATIC_DRAW);

    glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matambLoc);

    // Buffer de component difusa
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(matdiff), matdiff, GL_STATIC_DRAW);

    glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matdiffLoc);

    // Buffer de component especular
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(matspec), matspec, GL_STATIC_DRAW);

    glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matspecLoc);

    // Buffer de component shininness
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(matshin), matshin, GL_STATIC_DRAW);

    glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matshinLoc);

    glBindVertexArray(0);
}

void GLView::carregaShaders()
{
    // Creem els shaders per al fragment shader i el vertex shader
    QOpenGLShader fs (QOpenGLShader::Fragment, this);
    QOpenGLShader vs (QOpenGLShader::Vertex, this);
    // Carreguem el codi dels fitxers i els compilem
    fs.compileSourceFile("./shaders/basicLlumShader.frag");
    vs.compileSourceFile("./shaders/basicLlumShader.vert");
    // Creem el program
    program = new QOpenGLShaderProgram(this);
    // Li afegim els shaders corresponents
    program->addShader(&fs);
    program->addShader(&vs);
    // Linkem el program
    program->link();
    // Indiquem que aquest és el program que volem usar
    program->bind();

    // Obtenim identificador per a l'atribut “vertex” del vertex shader
    vertexLoc = glGetAttribLocation (program->programId(), "vertex");
    // Obtenim identificador per a l'atribut “normal” del vertex shader
    normalLoc = glGetAttribLocation (program->programId(), "normal");
    // Obtenim identificador per a l'atribut “matamb” del vertex shader
    matambLoc = glGetAttribLocation (program->programId(), "matamb");
    // Obtenim identificador per a l'atribut “matdiff” del vertex shader
    matdiffLoc = glGetAttribLocation (program->programId(), "matdiff");
    // Obtenim identificador per a l'atribut “matspec” del vertex shader
    matspecLoc = glGetAttribLocation (program->programId(), "matspec");
    // Obtenim identificador per a l'atribut “matshin” del vertex shader
    matshinLoc = glGetAttribLocation (program->programId(), "matshin");

    // Demanem identificadors per als uniforms del vertex shader
    transLoc = glGetUniformLocation (program->programId(), "TG");
    projLoc = glGetUniformLocation (program->programId(), "proj");
    viewLoc = glGetUniformLocation (program->programId(), "view");
    
    colFocusLoc = glGetUniformLocation (program->programId(), "colFocus");
    llumAmbLoc  = glGetUniformLocation (program->programId(), "llumAmbient");
    posFocusLoc = glGetUniformLocation (program->programId(), "posFocus");
    
    normalMatLoc = glGetUniformLocation (program->programId(), "NormalMatrix");
    
    compLoc = glGetUniformLocation (program->programId(), "components");
    greyCoefLoc = glGetUniformLocation (program->programId(), "greyCoef");
}

