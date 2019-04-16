# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'interface.ui'
#
# Created by: PyQt5 UI code generator 5.11.3
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(687, 593)
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.stepButton = QtWidgets.QPushButton(self.centralwidget)
        self.stepButton.setGeometry(QtCore.QRect(580, 70, 91, 28))
        self.stepButton.setObjectName("stepButton")
        self.loadProgramButton = QtWidgets.QPushButton(self.centralwidget)
        self.loadProgramButton.setGeometry(QtCore.QRect(30, 30, 91, 28))
        self.loadProgramButton.setObjectName("loadProgramButton")
        self.memCombo_1 = QtWidgets.QComboBox(self.centralwidget)
        self.memCombo_1.setGeometry(QtCore.QRect(240, 110, 101, 22))
        self.memCombo_1.setObjectName("memCombo_1")
        self.memList_1 = QtWidgets.QTableWidget(self.centralwidget)
        self.memList_1.setGeometry(QtCore.QRect(20, 141, 321, 401))
        self.memList_1.setObjectName("memList_1")
        self.memList_1.setColumnCount(0)
        self.memList_1.setRowCount(0)
        self.completeButton = QtWidgets.QPushButton(self.centralwidget)
        self.completeButton.setGeometry(QtCore.QRect(470, 70, 93, 28))
        self.completeButton.setObjectName("completeButton")
        self.memCombo_2 = QtWidgets.QComboBox(self.centralwidget)
        self.memCombo_2.setGeometry(QtCore.QRect(570, 110, 101, 22))
        self.memCombo_2.setObjectName("memCombo_2")
        self.cacheOn = QtWidgets.QCheckBox(self.centralwidget)
        self.cacheOn.setGeometry(QtCore.QRect(470, 30, 81, 20))
        self.cacheOn.setObjectName("cacheOn")
        self.pipelineOn = QtWidgets.QCheckBox(self.centralwidget)
        self.pipelineOn.setGeometry(QtCore.QRect(580, 30, 81, 20))
        self.pipelineOn.setObjectName("pipelineOn")
        self.memList_2 = QtWidgets.QTableWidget(self.centralwidget)
        self.memList_2.setGeometry(QtCore.QRect(350, 140, 321, 401))
        self.memList_2.setObjectName("memList_2")
        self.memList_2.setColumnCount(0)
        self.memList_2.setRowCount(0)
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 687, 26))
        self.menubar.setObjectName("menubar")
        self.menuHello_Test = QtWidgets.QMenu(self.menubar)
        self.menuHello_Test.setObjectName("menuHello_Test")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)
        self.actionLoad_Program_to_Memory = QtWidgets.QAction(MainWindow)
        self.actionLoad_Program_to_Memory.setEnabled(True)
        self.actionLoad_Program_to_Memory.setObjectName("actionLoad_Program_to_Memory")
        self.menuHello_Test.addAction(self.actionLoad_Program_to_Memory)
        self.menuHello_Test.addSeparator()
        self.menubar.addAction(self.menuHello_Test.menuAction())

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "MainWindow"))
        self.stepButton.setText(_translate("MainWindow", "Step"))
        self.loadProgramButton.setText(_translate("MainWindow", "Load Program"))
        self.completeButton.setText(_translate("MainWindow", "Complete"))
        self.cacheOn.setText(_translate("MainWindow", "Cache"))
        self.pipelineOn.setText(_translate("MainWindow", "Pipeline"))
        self.menuHello_Test.setTitle(_translate("MainWindow", "Edit"))
        self.actionLoad_Program_to_Memory.setText(_translate("MainWindow", "Load Program to Memory"))

