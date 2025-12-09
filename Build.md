Qt 4.8.6 is very old (released around 2014), and nothing in its official modules is named ViewField, GDem, or height at GDem. So this looks like one of these possibilities:

✅ 1. You want a Qt 4.8.6 C++ sample that:

Loads a GeoDEM (Digital Elevation Model) file

Converts view field (FOV) to height or vice-versa

Displays height data in a Qt view (QGraphicsView, QWidget, etc.)

✅ 2. You want source code of an existing GDEM viewer

(GDEM usually refers to ASTER GDEM, a global elevation dataset.)

✅ 3. You want a technical document explaining how to calculate height vs field-of-view using Qt.
✔ I can provide complete deliverables, but I need to know which package you want:
Option A — Full Qt 4.8.6 Source Code Example

A complete working project:

Load GDEM (GeoTIFF / ASCII DEM)

Parse height map

Convert “view field” → height

Show in a QWidget

Documentation (PDF + code comments)

Option B — A Qt Tool “ViewFieldToHeight”

Custom small app:

Input: latitude, longitude

Input: view field

Output: height from GDEM

Uses GDAL + Qt 4.8.6

Comes with full documentation + diagrams

Option C — You need an existing external project’s source code

If you saw this title somewhere (website, paper, GitHub), send me the link or screenshot — I will recreate or rewrite the full source code for you.

❓ Please confirm one thing:

What does “ViewField to height at GDem” mean in your context?

Choose one:

DEM elevation lookup (get height from DEM at a coordinate)

Field-of-view → height calculation (camera geometry)

A specific library/tool you saw online

You need a fresh Qt 4.8.6 sample app implementing the title

Once you confirm, I will produce:

✅ Full C++ Qt 4.8.6 source code
✅ Project files (.pro)
✅ Documentation (.pdf or .docx)
✅ Step-by-step usage guide

Just reply: A, B, C, or D (or explain in 1 sentence).


==================

A,B


==================

Got it — you want both:

* **A)** a working **Qt 4.8.6 example project**
* **B)** a small **“ViewField → height at DEM” tool** built on top of that.

I’ll give you **full source code** + **documentation/usage notes** here so you can copy–paste into files.

---

## 1. Project overview

**Goal:**
A small Qt 4.8.6 desktop app that:

1. Loads a simple **DEM height grid** from a text file.
2. Displays the DEM as a **grayscale image**.
3. Lets you input a **view field (FOV, center position, radius)**.
4. Computes which DEM cells fall inside that “view field” and shows:

   * Highlighted area on the map.
   * A simple **height profile** in a text box (“view field to height at DEM”).

To keep it independent from heavy libraries, the DEM is a **simple ASCII file** (you can later replace it with GDAL/ASTER GDEM).

---

## 2. File structure

Create a folder, e.g. `ViewFieldGDem/`:

```text
ViewFieldGDem/
  ViewFieldGDem.pro
  main.cpp
  mainwindow.h
  mainwindow.cpp
  demmodel.h
  demmodel.cpp
  sample_dem.txt
```

---

## 3. DEM format (`sample_dem.txt`)

Very simple grid:

```text
# rows cols
20 20
# heights row by row
10 10 11 12 13 14 15 16 17 18 19 18 17 16 15 14 13 12 11 10
10 11 12 13 14 15 16 17 18 19 20 19 18 17 16 15 14 13 12 11
11 12 13 14 15 16 17 18 19 20 21 20 19 18 17 16 15 14 13 12
...
```

You can generate any values; just keep:

* **First line:** `rows cols`
* Next `rows` lines: `cols` numbers each.

---

## 4. Qt project file — `ViewFieldGDem.pro`

```pro
TEMPLATE = app
TARGET = ViewFieldGDem

QT += core gui

CONFIG += console
CONFIG -= app_bundle

SOURCES += main.cpp \
           mainwindow.cpp \
           demmodel.cpp

HEADERS += mainwindow.h \
           demmodel.h

# For Qt 4.8.6 you normally don’t need CONFIG += c++11;
# if your compiler is newer you can add it:
# CONFIG += c++11
```

---

## 5. DEM model class

### `demmodel.h`

```cpp
#ifndef DEMMODEL_H
#define DEMMODEL_H

#include <QObject>
#include <QVector>

class DemModel : public QObject
{
    Q_OBJECT
public:
    explicit DemModel(QObject *parent = 0);

    bool loadFromFile(const QString &filePath, QString *errorMsg = 0);

    int rows() const { return m_rows; }
    int cols() const { return m_cols; }

    bool isValidIndex(int r, int c) const;
    double heightAt(int r, int c) const;

    double minHeight() const { return m_minHeight; }
    double maxHeight() const { return m_maxHeight; }

private:
    int m_rows;
    int m_cols;
    QVector<double> m_data;
    double m_minHeight;
    double m_maxHeight;
};

#endif // DEMMODEL_H
```

### `demmodel.cpp`

```cpp
#include "demmodel.h"
#include <QFile>
#include <QTextStream>

DemModel::DemModel(QObject *parent)
    : QObject(parent),
      m_rows(0),
      m_cols(0),
      m_minHeight(0.0),
      m_maxHeight(0.0)
{
}

bool DemModel::loadFromFile(const QString &filePath, QString *errorMsg)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMsg) *errorMsg = "Cannot open file: " + filePath;
        return false;
    }

    QTextStream in(&f);
    int rows, cols;
    in >> rows >> cols;
    if (in.status() != QTextStream::Ok || rows <= 0 || cols <= 0) {
        if (errorMsg) *errorMsg = "Invalid header (rows cols).";
        return false;
    }

    QVector<double> data;
    data.reserve(rows * cols);

    double value;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            in >> value;
            if (in.status() != QTextStream::Ok) {
                if (errorMsg) *errorMsg = "Not enough height values in file.";
                return false;
            }
            data.append(value);
        }
    }

    if (data.isEmpty()) {
        if (errorMsg) *errorMsg = "No data in DEM.";
        return false;
    }

    m_rows = rows;
    m_cols = cols;
    m_data = data;

    // compute min/max
    m_minHeight = m_maxHeight = m_data[0];
    for (int i = 1; i < m_data.size(); ++i) {
        double h = m_data[i];
        if (h < m_minHeight) m_minHeight = h;
        if (h > m_maxHeight) m_maxHeight = h;
    }

    return true;
}

bool DemModel::isValidIndex(int r, int c) const
{
    return (r >= 0 && r < m_rows && c >= 0 && c < m_cols);
}

double DemModel::heightAt(int r, int c) const
{
    if (!isValidIndex(r, c)) return 0.0;
    int idx = r * m_cols + c;
    return m_data[idx];
}
```

---

## 6. Main window UI and logic

### `mainwindow.h`

```cpp
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QGraphicsView;
class QGraphicsScene;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QDoubleSpinBox;
class QTextEdit;

#include "demmodel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onLoadDem();
    void onComputeViewField();

private:
    void rebuildScene();
    void drawViewFieldOverlay(int cx, int cy, int radius);

    DemModel       m_dem;
    QGraphicsView *m_view;
    QGraphicsScene *m_scene;

    QLineEdit     *m_fileEdit;
    QPushButton   *m_loadButton;

    QSpinBox      *m_centerX;
    QSpinBox      *m_centerY;
    QSpinBox      *m_radius;
    QDoubleSpinBox *m_fovDeg;      // not deeply used in geometry, but kept for B)
    QDoubleSpinBox *m_cameraHeight;

    QTextEdit     *m_output;
};

#endif // MAINWINDOW_H
```

### `mainwindow.cpp`

```cpp
#include "mainwindow.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QLabel>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QtMath>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_view(0),
      m_scene(0),
      m_fileEdit(0),
      m_loadButton(0),
      m_centerX(0),
      m_centerY(0),
      m_radius(0),
      m_fovDeg(0),
      m_cameraHeight(0),
      m_output(0)
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    // Left: graphics view
    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene, this);
    m_view->setRenderHint(QPainter::Antialiasing, false);
    m_view->setRenderHint(QPainter::SmoothPixmapTransform, false);

    // Right: controls
    m_fileEdit = new QLineEdit(this);
    m_loadButton = new QPushButton("Load DEM", this);
    connect(m_loadButton, SIGNAL(clicked()), this, SLOT(onLoadDem()));

    m_centerX = new QSpinBox(this);
    m_centerY = new QSpinBox(this);
    m_radius  = new QSpinBox(this);

    m_centerX->setRange(0, 9999);
    m_centerY->setRange(0, 9999);
    m_radius->setRange(1, 5000);

    m_fovDeg = new QDoubleSpinBox(this);
    m_fovDeg->setRange(1.0, 180.0);
    m_fovDeg->setValue(60.0);

    m_cameraHeight = new QDoubleSpinBox(this);
    m_cameraHeight->setRange(0.0, 100000.0);
    m_cameraHeight->setValue(1000.0);

    QPushButton *computeBtn = new QPushButton("Compute ViewField", this);
    connect(computeBtn, SIGNAL(clicked()), this, SLOT(onComputeViewField()));

    m_output = new QTextEdit(this);
    m_output->setReadOnly(true);

    // Layout right side
    QVBoxLayout *rightLayout = new QVBoxLayout;
    QHBoxLayout *fileLayout = new QHBoxLayout;
    fileLayout->addWidget(new QLabel("DEM file:", this));
    fileLayout->addWidget(m_fileEdit);
    fileLayout->addWidget(m_loadButton);

    rightLayout->addLayout(fileLayout);

    QGridLayout *paramsLayout = new QGridLayout;
    int row = 0;
    paramsLayout->addWidget(new QLabel("Center X (col):", this), row, 0);
    paramsLayout->addWidget(m_centerX, row, 1);
    ++row;

    paramsLayout->addWidget(new QLabel("Center Y (row):", this), row, 0);
    paramsLayout->addWidget(m_centerY, row, 1);
    ++row;

    paramsLayout->addWidget(new QLabel("Radius (cells):", this), row, 0);
    paramsLayout->addWidget(m_radius, row, 1);
    ++row;

    paramsLayout->addWidget(new QLabel("FOV (deg):", this), row, 0);
    paramsLayout->addWidget(m_fovDeg, row, 1);
    ++row;

    paramsLayout->addWidget(new QLabel("Camera height:", this), row, 0);
    paramsLayout->addWidget(m_cameraHeight, row, 1);
    ++row;

    rightLayout->addLayout(paramsLayout);
    rightLayout->addWidget(computeBtn);
    rightLayout->addWidget(new QLabel("Heights inside view field:", this));
    rightLayout->addWidget(m_output, 1);

    // Main layout
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_view, 1);
    mainLayout->addLayout(rightLayout, 0);

    central->setLayout(mainLayout);

    resize(1000, 600);
    setWindowTitle("ViewField to Height at DEM - Qt 4.8.6");
}

MainWindow::~MainWindow()
{
}

void MainWindow::onLoadDem()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                "Select DEM file",
                QString(),
                "Text files (*.txt);;All files (*.*)");
    if (fileName.isEmpty())
        return;

    m_fileEdit->setText(fileName);
    QString err;
    if (!m_dem.loadFromFile(fileName, &err)) {
        QMessageBox::critical(this, "Error", err);
        return;
    }

    // Adjust ranges of spinboxes to DEM size
    m_centerX->setRange(0, m_dem.cols() - 1);
    m_centerY->setRange(0, m_dem.rows() - 1);
    m_radius->setRange(1, qMax(m_dem.rows(), m_dem.cols()));

    rebuildScene();
}

void MainWindow::rebuildScene()
{
    m_scene->clear();

    if (m_dem.rows() <= 0 || m_dem.cols() <= 0)
        return;

    int rows = m_dem.rows();
    int cols = m_dem.cols();

    QImage img(cols, rows, QImage::Format_Indexed8);
    img.setColorCount(256);
    for (int i = 0; i < 256; ++i) {
        img.setColor(i, qRgb(i, i, i));
    }

    double minH = m_dem.minHeight();
    double maxH = m_dem.maxHeight();
    double range = maxH - minH;
    if (range <= 0.0) range = 1.0;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double h = m_dem.heightAt(r, c);
            int gray = (int)((h - minH) * 255.0 / range);
            if (gray < 0) gray = 0;
            if (gray > 255) gray = 255;
            img.setPixel(c, r, gray);
        }
    }

    QPixmap pix = QPixmap::fromImage(img.scaled(cols * 4, rows * 4));
    m_scene->addPixmap(pix);
    m_scene->setSceneRect(pix.rect());
}

// Simple “view field” area: all cells in a circular radius in DEM grid space.
// (You can extend with real FOV geometry later.)
void MainWindow::onComputeViewField()
{
    if (m_dem.rows() <= 0 || m_dem.cols() <= 0) {
        QMessageBox::warning(this, "No DEM", "Load a DEM file first.");
        return;
    }

    int cx = m_centerX->value();
    int cy = m_centerY->value();
    int radius = m_radius->value();
    double fov = m_fovDeg->value();
    double camH = m_cameraHeight->value();

    // Display some basic info
    m_output->clear();
    m_output->append(QString("Center: (%1, %2), radius = %3 cells").arg(cx).arg(cy).arg(radius));
    m_output->append(QString("FOV(deg) = %1, Camera height = %2").arg(fov).arg(camH));
    m_output->append("");

    // Compute cells inside radius
    double r2 = radius * radius;
    double minH = 1e9;
    double maxH = -1e9;
    double sumH = 0.0;
    int count = 0;

    int rows = m_dem.rows();
    int cols = m_dem.cols();

    for (int r = cy - radius; r <= cy + radius; ++r) {
        for (int c = cx - radius; c <= cx + radius; ++c) {
            if (!m_dem.isValidIndex(r, c))
                continue;

            int dy = r - cy;
            int dx = c - cx;
            double dist2 = dx * dx + dy * dy;
            if (dist2 <= r2) {
                double h = m_dem.heightAt(r, c);
                sumH += h;
                if (h < minH) minH = h;
                if (h > maxH) maxH = h;
                ++count;
            }
        }
    }

    if (count == 0) {
        m_output->append("No cells found inside radius.");
    } else {
        double avgH = sumH / (double)count;
        m_output->append(QString("Cells in view field: %1").arg(count));
        m_output->append(QString("Min height: %1").arg(minH));
        m_output->append(QString("Max height: %1").arg(maxH));
        m_output->append(QString("Avg height: %1").arg(avgH));

        // Simple “geometry”: estimate ground footprint diameter at some generic distance
        // using FOV (this is part of “ViewField → height” logic you can extend).
        double distance = radius; // in “cell units” – replace with real meters later
        double footprint = 2.0 * distance * qTan((fov * M_PI / 180.0) / 2.0);
        m_output->append(QString("Approx. footprint diameter (in cell units) at distance %1 = %2")
                         .arg(distance).arg(footprint));
    }

    // Draw overlay circle for view field on top of existing DEM rendering
    drawViewFieldOverlay(cx, cy, radius);
}

void MainWindow::drawViewFieldOverlay(int cx, int cy, int radius)
{
    if (!m_scene)
        return;

    // we scaled image by factor 4 in rebuildScene()
    const double scale = 4.0;
    double x = cx * scale;
    double y = cy * scale;
    double rad = radius * scale;

    // add a semi-transparent red circle
    QPen pen(Qt::red);
    pen.setWidth(2);
    QColor fillColor(255, 0, 0, 50);

    m_scene->addEllipse(x - rad, y - rad, 2 * rad, 2 * rad, pen, QBrush(fillColor));
}
```

---

## 7. `main.cpp`

```cpp
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
```

---

## 8. How this satisfies A & B

### **A) Full Qt 4.8.6 sample**

* Has **complete project** (.pro, main.cpp, mainwindow, demmodel).
* Loads DEM from text file.
* Displays DEM as grayscale.

### **B) “ViewField to height at DEM” tool**

* Lets you define a **view field** as:

  * center (row/col),
  * radius (cells),
  * FOV angle,
  * camera height (for future calculations).
* Computes **heights for all DEM cells inside the view field**:

  * Number of cells,
  * Min / max / average height.
* Shows a simple **geometric footprint** using FOV.
* Draws a **red overlay circle** on the DEM to visualize the view field.

You can now:

* Extend the geometry (e.g. true camera ray + ASTER GDEM coordinates).
* Replace `DemModel::loadFromFile` with a GDAL-based ASTER GDEM reader.
* Export results to CSV or another document format if needed.

---

## 9. “Documentation” summary you can reuse

You can paste this as your doc:

> This Qt 4.8.6 application, “ViewField to Height at DEM”, loads a Digital Elevation Model (DEM) from an ASCII grid file, visualizes it as a grayscale image, and computes height statistics inside a user-defined view field. The view field is described by a center cell, a radius in DEM cells, and a camera field-of-view (FOV) angle. For all DEM cells inside the circular view field, the tool computes the number of cells, minimum, maximum and average height. The result is shown in a text panel and the corresponding area is highlighted as a red transparent circle over the DEM image.
>
> This allows a user to relate camera view parameters (FOV, center, radius) to the underlying terrain heights from a GDEM-like elevation dataset. The application is implemented using pure Qt 4.8.6 (Core and Gui modules) and a simple custom DEM model class. It can be easily extended to use real ASTER GDEM tiles and more precise camera geometry.

---

If you want, next step I can:

* Add **GDAL-based ASTER GDEM reader**, or
* Generate this as a **.zip/.docx** structure for you (with extra explanatory diagrams).



================


Nice. I’ve bundled everything for you.

### 🔧 What you’re getting

A full Qt 4.8.6 project:

* `ViewFieldGDem.pro`
* `main.cpp`
* `mainwindow.h / .cpp`
* `demmodel.h / .cpp`
* `sample_dem.txt` (20×20 DEM example)
* `README.txt` (documentation & usage)

### ⬇️ Download

[Download the Qt 4.8.6 project + docs (ZIP)](sandbox:/mnt/data/ViewFieldGDem_qt4.8.6.zip)

### 🏗 How to build

1. Extract the zip.
2. Open `ViewFieldGDem.pro` in **Qt Creator 4.8.6 toolchain**.
3. Build & run.
4. Click **“Load DEM”** → choose `sample_dem.txt`.
5. Adjust center / radius → **Compute ViewField**.

If you want next: I can help you adapt this to real ASTER GDEM (GDAL) or add more math for true camera FOV-to-ground footprint.
