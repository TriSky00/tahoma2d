// Copyright Joseph McCormack
// Inkframe pose reference panel implementation.

#include "posereferencepanel.h"

// Tnz6 includes
#include "pane.h"
#include "tapp.h"
#include "floatingpanelcommand.h"
#include "menubarcommandids.h"

// Qt includes
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QFileDialog>
#include <QApplication>
#include <QClipboard>
#include <QTimer>
#include <QDir>
#include <QtMath>

#include <cmath>

//=============================================================================
// Rigs. Y up, ground at y=0, sizes roughly in meters.
// offset = bind translation from the parent joint origin (parent space).
// bone   = the segment drawn from this joint's origin, in its local space;
//          children typically attach at the parent's bone endpoint.

namespace {

const PoseRefJointDef humanJoints[] = {
    // name        parent offset                     bone                      th
    {"Hips", -1, QVector3D(0.f, 0.98f, 0.f), QVector3D(0.f, 0.10f, 0.f), 0.095f},
    {"Spine", 0, QVector3D(0.f, 0.10f, 0.f), QVector3D(0.f, 0.14f, 0.f), 0.080f},
    {"Chest", 1, QVector3D(0.f, 0.14f, 0.f), QVector3D(0.f, 0.16f, 0.f), 0.105f},
    {"Neck", 2, QVector3D(0.f, 0.16f, 0.f), QVector3D(0.f, 0.06f, 0.f), 0.035f},
    {"Head", 3, QVector3D(0.f, 0.06f, 0.f), QVector3D(0.f, 0.20f, 0.f), 0.078f},
    {"L Arm", 2, QVector3D(0.17f, 0.13f, 0.f), QVector3D(0.f, -0.27f, 0.f), 0.040f},
    {"L Forearm", 5, QVector3D(0.f, -0.27f, 0.f), QVector3D(0.f, -0.25f, 0.f), 0.034f},
    {"L Hand", 6, QVector3D(0.f, -0.25f, 0.f), QVector3D(0.f, -0.16f, 0.f), 0.026f},
    {"R Arm", 2, QVector3D(-0.17f, 0.13f, 0.f), QVector3D(0.f, -0.27f, 0.f), 0.040f},
    {"R Forearm", 8, QVector3D(0.f, -0.27f, 0.f), QVector3D(0.f, -0.25f, 0.f), 0.034f},
    {"R Hand", 9, QVector3D(0.f, -0.25f, 0.f), QVector3D(0.f, -0.16f, 0.f), 0.026f},
    {"L Leg", 0, QVector3D(0.095f, 0.f, 0.f), QVector3D(0.f, -0.44f, 0.f), 0.055f},
    {"L Shin", 11, QVector3D(0.f, -0.44f, 0.f), QVector3D(0.f, -0.42f, 0.f), 0.045f},
    {"L Foot", 12, QVector3D(0.f, -0.42f, 0.f), QVector3D(0.f, -0.05f, 0.19f), 0.032f},
    {"R Leg", 0, QVector3D(-0.095f, 0.f, 0.f), QVector3D(0.f, -0.44f, 0.f), 0.055f},
    {"R Shin", 14, QVector3D(0.f, -0.44f, 0.f), QVector3D(0.f, -0.42f, 0.f), 0.045f},
    {"R Foot", 15, QVector3D(0.f, -0.42f, 0.f), QVector3D(0.f, -0.05f, 0.19f), 0.032f},
};

// Quadruped faces +X (side profile toward the default camera).
const PoseRefJointDef quadJoints[] = {
    {"Hips", -1, QVector3D(-0.32f, 0.62f, 0.f), QVector3D(0.28f, 0.02f, 0.f), 0.110f},
    {"Spine", 0, QVector3D(0.28f, 0.02f, 0.f), QVector3D(0.30f, 0.02f, 0.f), 0.120f},
    {"Chest", 1, QVector3D(0.30f, 0.02f, 0.f), QVector3D(0.16f, 0.03f, 0.f), 0.115f},
    {"Neck", 2, QVector3D(0.16f, 0.03f, 0.f), QVector3D(0.15f, 0.11f, 0.f), 0.060f},
    {"Head", 3, QVector3D(0.15f, 0.11f, 0.f), QVector3D(0.22f, 0.f, 0.f), 0.085f},
    {"Tail", 0, QVector3D(-0.03f, 0.05f, 0.f), QVector3D(-0.22f, 0.05f, 0.f), 0.030f},
    {"L Front Leg", 2, QVector3D(0.10f, -0.04f, 0.10f), QVector3D(0.f, -0.28f, 0.f), 0.050f},
    {"L Front Shin", 6, QVector3D(0.f, -0.28f, 0.f), QVector3D(0.f, -0.24f, 0.f), 0.042f},
    {"L Front Paw", 7, QVector3D(0.f, -0.24f, 0.f), QVector3D(0.10f, -0.03f, 0.f), 0.036f},
    {"R Front Leg", 2, QVector3D(0.10f, -0.04f, -0.10f), QVector3D(0.f, -0.28f, 0.f), 0.050f},
    {"R Front Shin", 9, QVector3D(0.f, -0.28f, 0.f), QVector3D(0.f, -0.24f, 0.f), 0.042f},
    {"R Front Paw", 10, QVector3D(0.f, -0.24f, 0.f), QVector3D(0.10f, -0.03f, 0.f), 0.036f},
    {"L Hind Leg", 0, QVector3D(-0.02f, -0.03f, 0.11f), QVector3D(0.f, -0.30f, 0.f), 0.058f},
    {"L Hind Shin", 12, QVector3D(0.f, -0.30f, 0.f), QVector3D(0.f, -0.26f, 0.f), 0.045f},
    {"L Hind Paw", 13, QVector3D(0.f, -0.26f, 0.f), QVector3D(0.10f, -0.03f, 0.f), 0.038f},
    {"R Hind Leg", 0, QVector3D(-0.02f, -0.03f, -0.11f), QVector3D(0.f, -0.30f, 0.f), 0.058f},
    {"R Hind Shin", 15, QVector3D(0.f, -0.30f, 0.f), QVector3D(0.f, -0.26f, 0.f), 0.045f},
    {"R Hind Paw", 16, QVector3D(0.f, -0.26f, 0.f), QVector3D(0.10f, -0.03f, 0.f), 0.038f},
};

const PoseRefRig rigs[] = {
    {"Human", humanJoints, int(sizeof(humanJoints) / sizeof(humanJoints[0])),
     QVector3D(0.f, 0.95f, 0.f), 2.8f},
    {"Quadruped", quadJoints, int(sizeof(quadJoints) / sizeof(quadJoints[0])),
     QVector3D(0.05f, 0.55f, 0.f), 2.6f},
};
const int rigCount = int(sizeof(rigs) / sizeof(rigs[0]));

//-----------------------------------------------------------------------------
// Poses: per-joint euler angles (degrees, pitch=x yaw=y roll=z), by joint name.

struct PoseEntry {
  const char *joint;
  float x, y, z;
};
struct PoseDef {
  const char *name;
  const PoseEntry *entries;
  int count;
};

// clang-format off
const PoseEntry humanStand[]  = {{"L Arm", 0, 0, 8}, {"R Arm", 0, 0, -8}};
const PoseEntry humanTPose[]  = {{"L Arm", 0, 0, 90}, {"R Arm", 0, 0, -90}};
const PoseEntry humanWalk[]   = {
    {"L Arm", -28, 0, 6},  {"R Arm", 28, 0, -6},
    {"L Forearm", -18, 0, 0}, {"R Forearm", -22, 0, 0},
    {"L Leg", 26, 0, 0},   {"L Shin", 12, 0, 0}, {"L Foot", -8, 0, 0},
    {"R Leg", -22, 0, 0},  {"R Shin", 28, 0, 0},
    {"Spine", 4, 0, 0},    {"Head", -4, 0, 0}};
const PoseEntry humanRun[]    = {
    {"L Arm", -55, 0, 6},  {"R Arm", 45, 0, -6},
    {"L Forearm", -48, 0, 0}, {"R Forearm", -55, 0, 0},
    {"L Leg", 55, 0, 0},   {"L Shin", 25, 0, 0}, {"L Foot", -12, 0, 0},
    {"R Leg", -35, 0, 0},  {"R Shin", 75, 0, 0}, {"R Foot", 15, 0, 0},
    {"Spine", 12, 0, 0},   {"Chest", 8, 0, 0},   {"Head", -14, 0, 0}};
const PoseEntry humanSit[]    = {
    {"L Leg", 85, 0, 0},   {"L Shin", -85, 0, 0},
    {"R Leg", 85, 0, 0},   {"R Shin", -85, 0, 0},
    {"L Arm", -25, 0, 8},  {"R Arm", -25, 0, -8},
    {"L Forearm", -35, 0, 0}, {"R Forearm", -35, 0, 0},
    {"Spine", -6, 0, 0}};
const PoseEntry humanPunch[]  = {
    {"Chest", 0, -35, 0},  {"Spine", 6, -12, 0}, {"Head", 0, 30, 0},
    {"R Arm", 80, -15, 0}, {"R Forearm", -12, 0, 0},
    {"L Arm", -35, 0, 25}, {"L Forearm", -95, 0, 0},
    {"L Leg", 18, 0, 0},   {"R Leg", -14, 0, 0}, {"R Shin", 22, 0, 0}};

const PoseEntry quadWalk[]    = {
    {"L Front Leg", 24, 0, 0}, {"L Front Shin", -10, 0, 0},
    {"R Front Leg", -20, 0, 0}, {"R Front Shin", 16, 0, 0},
    {"L Hind Leg", -22, 0, 0}, {"L Hind Shin", 14, 0, 0},
    {"R Hind Leg", 20, 0, 0},  {"R Hind Shin", -8, 0, 0},
    {"Neck", 6, 0, 0}, {"Tail", 8, 0, 0}};
const PoseEntry quadRun[]     = {
    {"L Front Leg", 45, 0, 0}, {"L Front Shin", -25, 0, 0},
    {"R Front Leg", 35, 0, 0}, {"R Front Shin", -15, 0, 0},
    {"L Hind Leg", -40, 0, 0}, {"L Hind Shin", 30, 0, 0},
    {"R Hind Leg", -32, 0, 0}, {"R Hind Shin", 24, 0, 0},
    {"Spine", -6, 0, 0}, {"Chest", -4, 0, 0},
    {"Neck", -10, 0, 0}, {"Tail", 20, 0, 0}};
const PoseEntry quadSit[]     = {
    {"Hips", -35, 0, 0}, {"Spine", 18, 0, 0}, {"Chest", 14, 0, 0},
    {"Neck", 12, 0, 0},
    {"L Hind Leg", 55, 0, 0}, {"L Hind Shin", -95, 0, 0},
    {"R Hind Leg", 55, 0, 0}, {"R Hind Shin", -95, 0, 0},
    {"L Front Leg", -18, 0, 0}, {"R Front Leg", -18, 0, 0},
    {"Tail", -14, 0, 0}};
const PoseEntry quadPounce[]  = {
    {"Hips", 14, 0, 0}, {"Spine", -10, 0, 0}, {"Chest", -8, 0, 0},
    {"Neck", -16, 0, 0}, {"Head", 6, 0, 0},
    {"L Front Leg", 65, 0, 0}, {"L Front Shin", -45, 0, 0},
    {"R Front Leg", 55, 0, 0}, {"R Front Shin", -35, 0, 0},
    {"L Hind Leg", -55, 0, 0}, {"L Hind Shin", 60, 0, 0},
    {"R Hind Leg", -55, 0, 0}, {"R Hind Shin", 60, 0, 0},
    {"Tail", 25, 0, 0}};
// clang-format on

#define POSE(n, a) \
  { n, a, int(sizeof(a) / sizeof(a[0])) }
#define POSE0(n) \
  { n, nullptr, 0 }

const PoseDef humanPoses[] = {POSE("Stand", humanStand), POSE("T-Pose", humanTPose),
                              POSE("Walk", humanWalk),   POSE("Run", humanRun),
                              POSE("Sit", humanSit),     POSE("Punch", humanPunch)};
const PoseDef quadPoses[]  = {POSE0("Stand"), POSE("Walk", quadWalk),
                              POSE("Run", quadRun), POSE("Sit", quadSit),
                              POSE("Pounce", quadPounce)};

const PoseDef *rigPoses(int rig) { return rig == 0 ? humanPoses : quadPoses; }
int rigPoseCount(int rig) {
  return rig == 0 ? int(sizeof(humanPoses) / sizeof(humanPoses[0]))
                  : int(sizeof(quadPoses) / sizeof(quadPoses[0]));
}

}  // namespace

//=============================================================================
// PoseRefViewport

PoseRefViewport::PoseRefViewport(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_rigIndex(0)
    , m_selected(-1)
    , m_yaw(24.f)
    , m_pitch(-10.f)
    , m_distance(rigs[0].cameraDistance)
    , m_target(rigs[0].cameraTarget)
    , m_orbiting(false)
    , m_panning(false)
    , m_draggingJoint(false) {
  m_rot.assign(rigs[0].jointCount, QQuaternion());
  setMinimumSize(160, 160);
  setFocusPolicy(Qt::WheelFocus);
  setToolTip(tr("Drag a joint to pose. Drag empty space to orbit, wheel to "
                "zoom, middle-drag to pan."));

  // Verification hook: INKFRAME_POSEREF_SNAPSHOT=<dir> writes one PNG per
  // figure/pose after startup, for headless visual checks.
  const QByteArray snapDir = qgetenv("INKFRAME_POSEREF_SNAPSHOT");
  if (!snapDir.isEmpty()) {
    QTimer::singleShot(1200, this, [this, snapDir]() {
      QDir().mkpath(QString::fromLocal8Bit(snapDir));
      for (int r = 0; r < rigCount; r++) {
        setRig(r);
        for (int p = 0; p < rigPoseCount(r); p++) {
          applyPose(p);
          QImage img = grabFramebuffer();
          img.save(QString::fromLocal8Bit(snapDir) + "/" + rigs[r].name + "_" +
                   rigPoses(r)[p].name + ".png");
        }
      }
      setRig(0);
    });
  }
}

//-----------------------------------------------------------------------------

void PoseRefViewport::setRig(int index) {
  if (index < 0 || index >= rigCount) return;
  m_rigIndex = index;
  m_rot.assign(rigs[index].jointCount, QQuaternion());
  m_selected = -1;
  m_target   = rigs[index].cameraTarget;
  m_distance = rigs[index].cameraDistance;
  update();
}

//-----------------------------------------------------------------------------

void PoseRefViewport::applyPose(int poseIndex) {
  if (poseIndex < 0 || poseIndex >= rigPoseCount(m_rigIndex)) return;
  resetPose();
  const PoseDef &pose    = rigPoses(m_rigIndex)[poseIndex];
  const PoseRefRig &rig  = rigs[m_rigIndex];
  for (int i = 0; i < pose.count; i++) {
    for (int j = 0; j < rig.jointCount; j++) {
      if (qstrcmp(rig.joints[j].name, pose.entries[i].joint) == 0) {
        m_rot[j] = QQuaternion::fromEulerAngles(
            pose.entries[i].x, pose.entries[i].y, pose.entries[i].z);
        break;
      }
    }
  }
  update();
}

//-----------------------------------------------------------------------------

void PoseRefViewport::resetPose() {
  m_rot.assign(rigs[m_rigIndex].jointCount, QQuaternion());
  update();
}

//-----------------------------------------------------------------------------

QStringList PoseRefViewport::poseNames() const {
  QStringList names;
  for (int p = 0; p < rigPoseCount(m_rigIndex); p++)
    names << QString(rigPoses(m_rigIndex)[p].name);
  return names;
}

//-----------------------------------------------------------------------------

void PoseRefViewport::initializeGL() {
  initializeOpenGLFunctions();
  glClearColor(0.94f, 0.94f, 0.95f, 1.f);
  glEnable(GL_DEPTH_TEST);
}

//-----------------------------------------------------------------------------

void PoseRefViewport::resizeGL(int w, int h) {
  m_proj.setToIdentity();
  m_proj.perspective(40.f, h > 0 ? float(w) / float(h) : 1.f, 0.05f, 100.f);
}

//-----------------------------------------------------------------------------

void PoseRefViewport::updateView() {
  m_view.setToIdentity();
  QVector3D dir(std::cos(qDegreesToRadians(m_pitch)) *
                    std::sin(qDegreesToRadians(m_yaw)),
                std::sin(qDegreesToRadians(m_pitch)),
                std::cos(qDegreesToRadians(m_pitch)) *
                    std::cos(qDegreesToRadians(m_yaw)));
  QVector3D eye = m_target + dir * m_distance;
  m_view.lookAt(eye, m_target, QVector3D(0, 1, 0));
}

//-----------------------------------------------------------------------------

void PoseRefViewport::computeWorlds(std::vector<QMatrix4x4> &out) const {
  const PoseRefRig &rig = rigs[m_rigIndex];
  out.resize(rig.jointCount);
  for (int i = 0; i < rig.jointCount; i++) {
    QMatrix4x4 local;
    local.translate(rig.joints[i].offset);
    local.rotate(m_rot[i]);
    out[i] = (rig.joints[i].parent >= 0) ? out[rig.joints[i].parent] * local
                                         : local;
  }
}

//-----------------------------------------------------------------------------

void PoseRefViewport::drawBox(const QMatrix4x4 &m, float sx, float sy,
                              float sz) {
  glPushMatrix();
  glMultMatrixf(m.constData());
  glScalef(sx, sy, sz);
  static const float f[6][4][3] = {
      {{-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}},      // +z
      {{1, -1, -1}, {-1, -1, -1}, {-1, 1, -1}, {1, 1, -1}},  // -z
      {{1, -1, 1}, {1, -1, -1}, {1, 1, -1}, {1, 1, 1}},      // +x
      {{-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}, {-1, 1, -1}},  // -x
      {{-1, 1, 1}, {1, 1, 1}, {1, 1, -1}, {-1, 1, -1}},      // +y
      {{-1, -1, -1}, {1, -1, -1}, {1, -1, 1}, {-1, -1, 1}},  // -y
  };
  static const float n[6][3] = {{0, 0, 1}, {0, 0, -1}, {1, 0, 0},
                                {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}};
  glBegin(GL_QUADS);
  for (int i = 0; i < 6; i++) {
    glNormal3fv(n[i]);
    for (int v = 0; v < 4; v++) glVertex3fv(f[i][v]);
  }
  glEnd();
  glPopMatrix();
}

//-----------------------------------------------------------------------------

void PoseRefViewport::drawGrid() {
  glDisable(GL_LIGHTING);
  glColor3f(0.82f, 0.82f, 0.84f);
  glBegin(GL_LINES);
  for (int i = -4; i <= 4; i++) {
    glVertex3f(float(i) * 0.5f, 0.f, -2.f);
    glVertex3f(float(i) * 0.5f, 0.f, 2.f);
    glVertex3f(-2.f, 0.f, float(i) * 0.5f);
    glVertex3f(2.f, 0.f, float(i) * 0.5f);
  }
  glEnd();
}

//-----------------------------------------------------------------------------

void PoseRefViewport::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  updateView();

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(m_proj.constData());
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(m_view.constData());

  drawGrid();

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_NORMALIZE);
  const float lightPos[4] = {2.f, 4.f, 3.f, 0.f};
  const float lightDif[4] = {0.85f, 0.85f, 0.85f, 1.f};
  const float lightAmb[4] = {0.35f, 0.35f, 0.38f, 1.f};
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDif);
  glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);

  const PoseRefRig &rig = rigs[m_rigIndex];
  std::vector<QMatrix4x4> worlds;
  computeWorlds(worlds);

  for (int i = 0; i < rig.jointCount; i++) {
    const PoseRefJointDef &j = rig.joints[i];
    QVector3D bone           = j.bone;
    float len                = bone.length();
    if (len > 1e-5f) {
      QMatrix4x4 m = worlds[i];
      m.translate(bone * 0.5f);
      m.rotate(QQuaternion::rotationTo(QVector3D(0, 1, 0), bone.normalized()));
      glColor3f(0.66f, 0.68f, 0.72f);
      drawBox(m, j.thickness, len * 0.5f, j.thickness);
    }
    // joint marker
    QMatrix4x4 jm = worlds[i];
    if (i == m_selected)
      glColor3f(0.20f, 0.75f, 0.66f);
    else
      glColor3f(0.45f, 0.47f, 0.52f);
    drawBox(jm, j.thickness * 0.55f, j.thickness * 0.55f, j.thickness * 0.55f);
  }
  glDisable(GL_LIGHTING);
}

//-----------------------------------------------------------------------------

int PoseRefViewport::pickJoint(const QPoint &pos) {
  updateView();
  const PoseRefRig &rig = rigs[m_rigIndex];
  std::vector<QMatrix4x4> worlds;
  computeWorlds(worlds);
  QMatrix4x4 vp = m_proj * m_view;
  int best = -1;
  float bestDist = 16.f * float(devicePixelRatioF());
  for (int i = 0; i < rig.jointCount; i++) {
    QVector3D p = worlds[i].map(QVector3D(0, 0, 0));
    QVector3D c = vp.map(p);
    if (c.z() > 1.f) continue;
    float sx = (c.x() * 0.5f + 0.5f) * float(width());
    float sy = (1.f - (c.y() * 0.5f + 0.5f)) * float(height());
    float d  = std::hypot(sx - float(pos.x()), sy - float(pos.y()));
    if (d < bestDist) {
      bestDist = d;
      best     = i;
    }
  }
  return best;
}

//-----------------------------------------------------------------------------

void PoseRefViewport::mousePressEvent(QMouseEvent *e) {
  m_lastPos = e->pos();
  if (e->button() == Qt::MiddleButton) {
    m_panning = true;
    return;
  }
  if (e->button() == Qt::LeftButton) {
    int hit    = pickJoint(e->pos());
    m_selected = hit;
    if (hit >= 0)
      m_draggingJoint = true;
    else
      m_orbiting = true;
    update();
  }
}

//-----------------------------------------------------------------------------

void PoseRefViewport::mouseMoveEvent(QMouseEvent *e) {
  QPoint d  = e->pos() - m_lastPos;
  m_lastPos = e->pos();

  if (m_draggingJoint && m_selected >= 0) {
    const PoseRefRig &rig = rigs[m_rigIndex];
    std::vector<QMatrix4x4> worlds;
    computeWorlds(worlds);
    updateView();
    QMatrix4x4 viewInv = m_view.inverted();
    QVector3D camRight = viewInv.mapVector(QVector3D(1, 0, 0));
    QVector3D camUp    = viewInv.mapVector(QVector3D(0, 1, 0));
    QQuaternion worldDelta =
        QQuaternion::fromAxisAndAngle(camUp, float(d.x()) * 0.6f) *
        QQuaternion::fromAxisAndAngle(camRight, float(d.y()) * 0.6f);
    int parent = rig.joints[m_selected].parent;
    QQuaternion parentWorldRot;
    if (parent >= 0) {
      // extract rotation from the parent world matrix column vectors
      QMatrix4x4 pw = worlds[parent];
      QVector3D x = pw.mapVector(QVector3D(1, 0, 0)).normalized();
      QVector3D y = pw.mapVector(QVector3D(0, 1, 0)).normalized();
      QVector3D z = pw.mapVector(QVector3D(0, 0, 1)).normalized();
      parentWorldRot = QQuaternion::fromAxes(x, y, z);
    }
    QQuaternion localDelta = parentWorldRot.inverted() * worldDelta *
                             parentWorldRot;
    m_rot[m_selected] = localDelta * m_rot[m_selected];
    update();
    return;
  }
  if (m_orbiting) {
    m_yaw -= float(d.x()) * 0.5f;
    m_pitch = qBound(-89.f, m_pitch - float(d.y()) * 0.4f, 89.f);
    update();
    return;
  }
  if (m_panning) {
    updateView();
    QMatrix4x4 viewInv = m_view.inverted();
    QVector3D camRight = viewInv.mapVector(QVector3D(1, 0, 0));
    QVector3D camUp    = viewInv.mapVector(QVector3D(0, 1, 0));
    float scale        = m_distance * 0.0016f;
    m_target += camRight * (-float(d.x()) * scale) + camUp * (float(d.y()) * scale);
    update();
  }
}

//-----------------------------------------------------------------------------

void PoseRefViewport::mouseReleaseEvent(QMouseEvent *) {
  m_orbiting = m_panning = m_draggingJoint = false;
}

//-----------------------------------------------------------------------------

void PoseRefViewport::wheelEvent(QWheelEvent *e) {
  float steps = float(e->angleDelta().y()) / 120.f;
  m_distance  = qBound(0.6f, m_distance * std::pow(0.9f, steps), 12.f);
  update();
}

//-----------------------------------------------------------------------------

QMatrix4x4 PoseRefViewport::jointWorld(
    int index, const std::vector<QMatrix4x4> &cache) const {
  return cache[index];
}

//=============================================================================
// PoseReferencePane

PoseReferencePane::PoseReferencePane(QWidget *parent) : QWidget(parent) {
  m_viewport            = new PoseRefViewport(this);
  m_figureCombo         = new QComboBox(this);
  m_poseCombo           = new QComboBox(this);
  QPushButton *resetBtn = new QPushButton(tr("Reset"), this);
  QPushButton *saveBtn  = new QPushButton(tr("Save PNG..."), this);
  QPushButton *copyBtn  = new QPushButton(tr("Copy"), this);

  for (int r = 0; r < rigCount; r++) m_figureCombo->addItem(tr(rigs[r].name));
  m_poseCombo->addItems(m_viewport->poseNames());

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(2, 2, 2, 2);
  mainLayout->setSpacing(2);
  QHBoxLayout *bar = new QHBoxLayout();
  bar->setSpacing(4);
  bar->addWidget(m_figureCombo);
  bar->addWidget(m_poseCombo, 1);
  bar->addWidget(resetBtn);
  bar->addWidget(copyBtn);
  bar->addWidget(saveBtn);
  mainLayout->addLayout(bar);
  mainLayout->addWidget(m_viewport, 1);

  connect(m_figureCombo, SIGNAL(currentIndexChanged(int)),
          SLOT(onFigureChanged(int)));
  connect(m_poseCombo, SIGNAL(activated(int)), SLOT(onPoseChanged(int)));
  connect(resetBtn, SIGNAL(clicked()), SLOT(onResetPose()));
  connect(saveBtn, SIGNAL(clicked()), SLOT(onSavePng()));
  connect(copyBtn, SIGNAL(clicked()), SLOT(onCopyImage()));
}

//-----------------------------------------------------------------------------

void PoseReferencePane::onFigureChanged(int index) {
  m_viewport->setRig(index);
  m_poseCombo->clear();
  m_poseCombo->addItems(m_viewport->poseNames());
}

void PoseReferencePane::onPoseChanged(int index) {
  m_viewport->applyPose(index);
}

void PoseReferencePane::onResetPose() { m_viewport->resetPose(); }

void PoseReferencePane::onSavePng() {
  QString fp = QFileDialog::getSaveFileName(this, tr("Save Pose Snapshot"),
                                            QString(), "PNG (*.png)");
  if (fp.isEmpty()) return;
  if (!fp.endsWith(".png", Qt::CaseInsensitive)) fp += ".png";
  m_viewport->grabFramebuffer().save(fp);
}

void PoseReferencePane::onCopyImage() {
  QApplication::clipboard()->setImage(m_viewport->grabFramebuffer());
}

//=============================================================================
// Panel registration

class PoseReferencePanelFactory final : public TPanelFactory {
public:
  PoseReferencePanelFactory() : TPanelFactory("PoseReference") {}
  void initialize(TPanel *panel) override {
    PoseReferencePane *pane = new PoseReferencePane(panel);
    panel->setWidget(pane);
    panel->setWindowTitle(QObject::tr("Pose Reference"));
    panel->setIsMaximizable(false);
    panel->getTitleBar()->showTitleBar(TApp::instance()->getShowTitleBars());
    QObject::connect(TApp::instance(), SIGNAL(showTitleBars(bool)),
                     panel->getTitleBar(), SLOT(showTitleBar(bool)));
  }
} poseReferencePanelFactory;

OpenFloatingPanel openPoseReferencePanelCommand(MI_OpenPoseReference,
                                                "PoseReference",
                                                QObject::tr("Pose Reference"));
