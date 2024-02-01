#include "LL4GLWidget.h"

#include <iostream>

LL4GLWidget::LL4GLWidget (QWidget* parent) : QOpenGLWidget(parent), program(NULL)
{
  setFocusPolicy(Qt::StrongFocus);  // per rebre events de teclat
  xClick = yClick = 0;
  DoingInteractive = NONE;
}

LL4GLWidget::~LL4GLWidget ()
{
  if (program != NULL)
    delete program;
}

void LL4GLWidget::initializeGL ()
{
  // Cal inicialitzar l'ús de les funcions d'OpenGL
  initializeOpenGLFunctions();  

  glClearColor(0.5, 0.7, 1.0, 1.0); // defineix color de fons (d'esborrat)
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  
  carregaShaders();
  
  creaBuffersTrack();
  creaBuffersFantasma();
  creaBuffersMineCart();
  creaBuffersTerra();

  iniEscena();
  iniCamera();
}

void LL4GLWidget::iniMaterialTerra ()
{
  // Donem valors al material del terra
  amb = glm::vec3(0.0,0.0,1.0);
  diff = glm::vec3(0.0,0.0,8.0);
  spec = glm::vec3(0.8,0.8,0.8);
  shin = 100;

}

void LL4GLWidget::iniEscena ()
{
  centreEsc = glm::vec3(5,3,5);
  radiEsc = 8;  
}

void LL4GLWidget::iniCamera ()
{
  angleY = 0.0;

  viewTransform ();
}

void LL4GLWidget::paintGL () 
{
  // En cas de voler canviar els paràmetres del viewport, descomenteu la crida següent i
  // useu els paràmetres que considereu (els que hi ha són els de per defecte)
  // glViewport (0, 0, ample, alt);
  
  // Esborrem el frame-buffer i el depth-buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // TERRA
  glBindVertexArray (VAO_Terra);
  modelTransformTerra ();
  glDrawArrays(GL_TRIANGLES, 0, 6);
  
  // TRACK
  glBindVertexArray (VAO_Track);
  modelTransformTrack (7,0);
  glDrawArrays(GL_TRIANGLES, 0, track.faces().size()*3);
  
  // FANTASMA
  glBindVertexArray (VAO_Fantasma);
  modelTransformFantasma ();
  glDrawArrays(GL_TRIANGLES, 0, fantasma.faces().size()*3);

  // MINE CART
  glBindVertexArray (VAO_MineCart);
  modelTransformMineCart ();
  glDrawArrays(GL_TRIANGLES, 0, mineCart.faces().size()*3);
  
  glBindVertexArray(0);
}

void LL4GLWidget::resizeGL (int w, int h) 
{
#ifdef __APPLE__
  // Aquest codi és necessari únicament per a MACs amb pantalla retina.
  GLint vp[4];
  glGetIntegerv (GL_VIEWPORT, vp);
  ample = vp[2];
  alt = vp[3];
#else
  ample = w;
  alt = h;
#endif
  ra = float(ample)/float(alt);
  factorAngleY = M_PI / ample;

  projectTransform();
}

void LL4GLWidget::modelTransformTrack (float radi, float rotY)
{
  glm::mat4 TG;
  TG = glm::translate(glm::mat4(1.f), glm::vec3(radi,0,5));
  TG = glm::scale(TG, glm::vec3(escalaTrack, escalaTrack, escalaTrack));
  TG = glm::rotate(TG, float(glm::radians(rotY)), glm::vec3(0,1,0));
  TG = glm::translate(TG, -centreBaseTrack);
  
  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

void LL4GLWidget::modelTransformFantasma ()
{
  glm::mat4 TG;
  TG = glm::translate(glm::mat4(1.0f), glm::vec3(1,0.5,5));
  TG = glm::scale(TG, glm::vec3(escalaFantasma, escalaFantasma, escalaFantasma));
  TG = glm::translate(TG, -centreBaseFantasma);
  
  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

void LL4GLWidget::modelTransformMineCart ()
{
  glm::mat4 TG;
  TG = glm::translate(glm::mat4(1.f), glm::vec3(4,0,5));
  TG = glm::rotate(TG, float(glm::radians(90.0)), glm::vec3(0,1,0));
  TG = glm::scale(TG, glm::vec3(escalaMineCart, escalaMineCart, escalaMineCart));
  TG = glm::translate(TG, -centreBaseMineCart);
  
  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

void LL4GLWidget::modelTransformTerra ()
{
  glm::mat4 TG = glm::mat4(1.f);  // Matriu de transformació
  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

void LL4GLWidget::projectTransform ()
{
  glm::mat4 Proj;  // Matriu de projecció
  Proj = glm::perspective(float(M_PI/3.0), ra, radiEsc, 3.0f*radiEsc);

  glUniformMatrix4fv (projLoc, 1, GL_FALSE, &Proj[0][0]);
}

void LL4GLWidget::viewTransform ()
{
  View = glm::translate(glm::mat4(1.f), glm::vec3(0, 0, -2*radiEsc));
  View = glm::rotate(View, -angleY, glm::vec3(0, 1, 0));
  View = glm::translate(View, -centreEsc);

  glUniformMatrix4fv (viewLoc, 1, GL_FALSE, &View[0][0]);
}

void LL4GLWidget::keyPressEvent(QKeyEvent* event) 
{
  makeCurrent();
  switch (event->key()) {
    default: event->ignore(); break;
  }
  update();
}

void LL4GLWidget::mousePressEvent (QMouseEvent *e)
{
  xClick = e->x();
  yClick = e->y();

  if (e->button() & Qt::LeftButton &&
      ! (e->modifiers() & (Qt::ShiftModifier|Qt::AltModifier|Qt::ControlModifier)))
  {
    DoingInteractive = ROTATE;
  }
}

void LL4GLWidget::mouseReleaseEvent( QMouseEvent *)
{
  DoingInteractive = NONE;
}

void LL4GLWidget::calculaCapsaModelTrack ()
{
  // Càlcul capsa contenidora i valors transformacions inicials
  float minx, miny, minz, maxx, maxy, maxz;
  minx = maxx = track.vertices()[0];
  miny = maxy = track.vertices()[1];
  minz = maxz = track.vertices()[2];
  for (unsigned int i = 3; i < track.vertices().size(); i+=3)
  {
    if (track.vertices()[i+0] < minx)
      minx = track.vertices()[i+0];
    if (track.vertices()[i+0] > maxx)
      maxx = track.vertices()[i+0];
    if (track.vertices()[i+1] < miny)
      miny = track.vertices()[i+1];
    if (track.vertices()[i+1] > maxy)
      maxy = track.vertices()[i+1];
    if (track.vertices()[i+2] < minz)
      minz = track.vertices()[i+2];
    if (track.vertices()[i+2] > maxz)
      maxz = track.vertices()[i+2];
  }
  
  escalaTrack = 2.5/(maxz-minz);
  
  centreBaseTrack[0] = (minx+maxx)/2.0; 
  centreBaseTrack[1] = miny; 
  centreBaseTrack[2] = (minz+maxz)/2.0;
}

void LL4GLWidget::creaBuffersTrack ()
{
  // Carreguem el model de l'OBJ - Atenció! Abans de crear els buffers!
  track.load("./models/Track.obj");

  // Calculem la capsa contenidora del model
  calculaCapsaModelTrack ();
  
  // Creació del Vertex Array Object del Track
  glGenVertexArrays(1, &VAO_Track);
  glBindVertexArray(VAO_Track);

  // Creació dels buffers del model fantasma
  GLuint VBO_Track[6];
  // Buffer de posicions
  glGenBuffers(6, VBO_Track);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Track[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*track.faces().size()*3*3, track.VBO_vertices(), GL_STATIC_DRAW);

  // Activem l'atribut vertexLoc
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  // Buffer de normals
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Track[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*track.faces().size()*3*3, track.VBO_normals(), GL_STATIC_DRAW);

  glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(normalLoc);

  // En lloc del color, ara passem tots els paràmetres dels materials
  // Buffer de component ambient
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Track[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*track.faces().size()*3*3, track.VBO_matamb(), GL_STATIC_DRAW);

  glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matambLoc);

  // Buffer de component difusa
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Track[3]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*track.faces().size()*3*3, track.VBO_matdiff(), GL_STATIC_DRAW);

  glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matdiffLoc);

  // Buffer de component especular
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Track[4]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*track.faces().size()*3*3, track.VBO_matspec(), GL_STATIC_DRAW);

  glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matspecLoc);

  // Buffer de component shininness
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Track[5]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*track.faces().size()*3, track.VBO_matshin(), GL_STATIC_DRAW);

  glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matshinLoc);

  glBindVertexArray(0);
}

void LL4GLWidget::calculaCapsaModelFantasma ()
{
  // Càlcul capsa contenidora i valors transformacions inicials
  float minx, miny, minz, maxx, maxy, maxz;
  minx = maxx = fantasma.vertices()[0];
  miny = maxy = fantasma.vertices()[1];
  minz = maxz = fantasma.vertices()[2];
  for (unsigned int i = 3; i < fantasma.vertices().size(); i+=3)
  {
    if (fantasma.vertices()[i+0] < minx)
      minx = fantasma.vertices()[i+0];
    if (fantasma.vertices()[i+0] > maxx)
      maxx = fantasma.vertices()[i+0];
    if (fantasma.vertices()[i+1] < miny)
      miny = fantasma.vertices()[i+1];
    if (fantasma.vertices()[i+1] > maxy)
      maxy = fantasma.vertices()[i+1];
    if (fantasma.vertices()[i+2] < minz)
      minz = fantasma.vertices()[i+2];
    if (fantasma.vertices()[i+2] > maxz)
      maxz = fantasma.vertices()[i+2];
  }
  escalaFantasma = 1.5/(maxy-miny);
  
  centreBaseFantasma[0] = (minx+maxx)/2.0; 
  centreBaseFantasma[1] = miny; 
  centreBaseFantasma[2] = (minz+maxz)/2.0;
}

void LL4GLWidget::creaBuffersFantasma ()
{
  // Carreguem el model de l'OBJ - Atenció! Abans de crear els buffers!
  fantasma.load("./models/Fantasma.obj");

  // Calculem la capsa contenidora del model
  calculaCapsaModelFantasma ();
  
  // Creació del Vertex Array Object del Fantasma
  glGenVertexArrays(1, &VAO_Fantasma);
  glBindVertexArray(VAO_Fantasma);

  // Creació dels buffers del model fantasma
  GLuint VBO_Fantasma[6];
  // Buffer de posicions
  glGenBuffers(6, VBO_Fantasma);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Fantasma[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*fantasma.faces().size()*3*3, fantasma.VBO_vertices(), GL_STATIC_DRAW);

  // Activem l'atribut vertexLoc
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  // Buffer de normals
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Fantasma[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*fantasma.faces().size()*3*3, fantasma.VBO_normals(), GL_STATIC_DRAW);

  glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(normalLoc);

  // En lloc del color, ara passem tots els paràmetres dels materials
  // Buffer de component ambient
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Fantasma[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*fantasma.faces().size()*3*3, fantasma.VBO_matamb(), GL_STATIC_DRAW);

  glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matambLoc);

  // Buffer de component difusa
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Fantasma[3]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*fantasma.faces().size()*3*3, fantasma.VBO_matdiff(), GL_STATIC_DRAW);

  glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matdiffLoc);

  // Buffer de component especular
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Fantasma[4]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*fantasma.faces().size()*3*3, fantasma.VBO_matspec(), GL_STATIC_DRAW);

  glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matspecLoc);

  // Buffer de component shininness
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Fantasma[5]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*fantasma.faces().size()*3, fantasma.VBO_matshin(), GL_STATIC_DRAW);

  glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matshinLoc);

  glBindVertexArray(0);
}

void LL4GLWidget::calculaCapsaModelMineCart ()
{
  // Càlcul capsa contenidora i valors transformacions inicials
  float minx, miny, minz, maxx, maxy, maxz;
  minx = maxx = mineCart.vertices()[0];
  miny = maxy = mineCart.vertices()[1];
  minz = maxz = mineCart.vertices()[2];
  for (unsigned int i = 3; i < mineCart.vertices().size(); i+=3)
  {
    if (mineCart.vertices()[i+0] < minx)
      minx = mineCart.vertices()[i+0];
    if (mineCart.vertices()[i+0] > maxx)
      maxx = mineCart.vertices()[i+0];
    if (mineCart.vertices()[i+1] < miny)
      miny = mineCart.vertices()[i+1];
    if (mineCart.vertices()[i+1] > maxy)
      maxy = mineCart.vertices()[i+1];
    if (mineCart.vertices()[i+2] < minz)
      minz = mineCart.vertices()[i+2];
    if (mineCart.vertices()[i+2] > maxz)
      maxz = mineCart.vertices()[i+2];
  }
  escalaMineCart = 1.5/(maxx-minx);
  
  centreBaseMineCart[0] = (minx+maxx)/2.0; 
  centreBaseMineCart[1] = miny; 
  centreBaseMineCart[2] = (minz+maxz)/2.0;
}

void LL4GLWidget::creaBuffersMineCart ()
{
  // Carreguem el model de l'OBJ - Atenció! Abans de crear els buffers!
  mineCart.load("./models/MineCart.obj");

  // Calculem la capsa contenidora del model
  calculaCapsaModelMineCart ();
  
  // Creació del Vertex Array Object de la bola disco
  glGenVertexArrays(1, &VAO_MineCart);
  glBindVertexArray(VAO_MineCart);

  // Creació dels buffers del model mineCart
  GLuint VBO_MineCart[6];
  // Buffer de posicions
  glGenBuffers(6, VBO_MineCart);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_MineCart[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*mineCart.faces().size()*3*3, mineCart.VBO_vertices(), GL_STATIC_DRAW);

  // Activem l'atribut vertexLoc
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  // Buffer de normals
  glBindBuffer(GL_ARRAY_BUFFER, VBO_MineCart[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*mineCart.faces().size()*3*3, mineCart.VBO_normals(), GL_STATIC_DRAW);

  glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(normalLoc);

  // En lloc del color, ara passem tots els paràmetres dels materials
  // Buffer de component ambient
  glBindBuffer(GL_ARRAY_BUFFER, VBO_MineCart[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*mineCart.faces().size()*3*3, mineCart.VBO_matamb(), GL_STATIC_DRAW);

  glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matambLoc);

  // Buffer de component difusa
  glBindBuffer(GL_ARRAY_BUFFER, VBO_MineCart[3]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*mineCart.faces().size()*3*3, mineCart.VBO_matdiff(), GL_STATIC_DRAW);

  glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matdiffLoc);

  // Buffer de component especular
  glBindBuffer(GL_ARRAY_BUFFER, VBO_MineCart[4]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*mineCart.faces().size()*3*3, mineCart.VBO_matspec(), GL_STATIC_DRAW);

  glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matspecLoc);

  // Buffer de component shininness
  glBindBuffer(GL_ARRAY_BUFFER, VBO_MineCart[5]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*mineCart.faces().size()*3, mineCart.VBO_matshin(), GL_STATIC_DRAW);

  glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matshinLoc);

  glBindVertexArray(0);
}


void LL4GLWidget::creaBuffersTerra ()
{
  // VBO amb la posició dels vèrtexs
  glm::vec3 posterra[6] = {
	glm::vec3(0.0, 0.0, 10.0),
	glm::vec3(10.0, 0.0, 10.0),
	glm::vec3(0.0, 0.0, 0.0),
	glm::vec3(0.0, 0.0, 0.0),
	glm::vec3(10.0, 0.0, 10.0),
	glm::vec3(10.0, 0.0, 0.0),
  }; 

  // VBO amb la normal de cada vèrtex
  glm::vec3 norm1 (0,1,0);
  glm::vec3 normterra[6] = {
	norm1, norm1, norm1, norm1, norm1, norm1 // la normal (0,1,0) per al terra
  };
  

  iniMaterialTerra();

  // Fem que el material del terra afecti a tots els vèrtexs per igual
  glm::vec3 matambterra[6] = {
	amb, amb, amb, amb, amb, amb
  };
  glm::vec3 matdiffterra[6] = {
	diff, diff, diff, diff, diff, diff
  };
  glm::vec3 matspecterra[6] = {
	spec, spec, spec, spec, spec, spec
  };
  float matshinterra[6] = {
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

  // En lloc del color, ara passem tots els paràmetres dels materials
  // Buffer de component ambient
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matambterra), matambterra, GL_STATIC_DRAW);

  glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matambLoc);

  // Buffer de component difusa
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[3]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matdiffterra), matdiffterra, GL_STATIC_DRAW);

  glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matdiffLoc);

  // Buffer de component especular
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[4]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matspecterra), matspecterra, GL_STATIC_DRAW);

  glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matspecLoc);

  // Buffer de component shininness
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[5]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matshinterra), matshinterra, GL_STATIC_DRAW);

  glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matshinLoc);

  glBindVertexArray(0);
}

void LL4GLWidget::carregaShaders()
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
  projLoc = glGetUniformLocation (program->programId(), "Proj");
  viewLoc = glGetUniformLocation (program->programId(), "View");
}


