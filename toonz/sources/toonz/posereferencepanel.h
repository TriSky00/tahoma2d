// Copyright Joseph McCormack
// Inkframe pose reference panel: a posable 3D mannequin (human or quadruped)
// docked beside the canvas as an anatomy/drawing reference.

#pragma once

#ifndef POSEREFERENCEPANEL_H
#define POSEREFERENCEPANEL_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QQuaternion>
#include <QVector3D>
#include <QMatrix4x4>
#include <QWidget>

#include <vector>

class QComboBox;

//=============================================================================
// Rig data

struct PoseRefJointDef {
  const char *name;
  int parent;        // index into the rig, -1 for root
  QVector3D offset;  // bind offset from parent joint origin (parent space)
  QVector3D bone;    // bone vector in this joint's local space
  float thickness;   // box half-thickness
};

struct PoseRefRig {
  const char *name;
  const PoseRefJointDef *joints;
  int jointCount;
  QVector3D cameraTarget;
  float cameraDistance;
};

//=============================================================================
// PoseRefViewport

class PoseRefViewport final : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

  int m_rigIndex;
  std::vector<QQuaternion> m_rot;  // per-joint local rotation
  int m_selected;                  // joint index or -1

  // camera
  float m_yaw, m_pitch, m_distance;
  QVector3D m_target;
  QMatrix4x4 m_proj, m_view;

  QPoint m_lastPos;
  bool m_orbiting, m_panning, m_draggingJoint;

public:
  PoseRefViewport(QWidget *parent = nullptr);

  int getRigIndex() const { return m_rigIndex; }
  void setRig(int index);
  void applyPose(int poseIndex);  // index into the current rig's pose list
  void resetPose();
  QStringList poseNames() const;

protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

  void mousePressEvent(QMouseEvent *e) override;
  void mouseMoveEvent(QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;
  void wheelEvent(QWheelEvent *e) override;

private:
  void updateView();
  QMatrix4x4 jointWorld(int index, const std::vector<QMatrix4x4> &cache) const;
  void computeWorlds(std::vector<QMatrix4x4> &out) const;
  int pickJoint(const QPoint &pos);
  void drawBox(const QMatrix4x4 &m, float sx, float sy, float sz);
  void drawGrid();
};

//=============================================================================
// PoseReferencePane

class PoseReferencePane final : public QWidget {
  Q_OBJECT

  PoseRefViewport *m_viewport;
  QComboBox *m_figureCombo;
  QComboBox *m_poseCombo;

public:
  PoseReferencePane(QWidget *parent = nullptr);

private slots:
  void onFigureChanged(int index);
  void onPoseChanged(int index);
  void onResetPose();
  void onSavePng();
  void onCopyImage();
};

#endif  // POSEREFERENCEPANEL_H
