import Python.assembler
import Python.cpu
import Python.isasim
import Python.mem

from PyQt5.QtWidgets import QMainWindow, QApplication, QFileDialog, QTableWidgetItem
from Python.dialog import Ui_MainWindow

mem_selections = ["", "Registers", "L1 Data", "L1 Code", "RAM"]

class AppWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        def loadfile():
            print(QFileDialog.getOpenFileName()[0])

        self.ui.loadProgramButton.clicked.connect(loadfile)
        self.ui.memCombo_1.addItems(mem_selections)
        self.ui.memCombo_2.addItems(mem_selections)
        #for

        self.ui.memList_1.verticalHeader().hide()
        self.ui.memList_2.verticalHeader().hide()

        def loadtable1():
            print(self.ui.memCombo_1.currentText())
            self.ui.memList_1.setColumnCount(2)
            self.ui.memList_1.setRowCount(10)
            self.ui.memList_1.setItem(0, 0, QTableWidgetItem("TEXT"))

        def loadtable2():
            print(self.ui.memCombo_2.currentText())

        self.ui.memCombo_1.currentIndexChanged.connect(loadtable1)
        self.ui.memCombo_2.currentIndexChanged.connect(loadtable2)

        def setcacheonoff():
            if(self.ui.cacheOn.isChecked()):

        self.ui.cacheOn.stateChanged.connect(setcacheonoff)
        def setpipelineonoff():
            print(self.ui.pipelineOn.isChecked())
        self.ui.pipelineOn.stateChanged.connect(setpipelineonoff)

        self.show()

app = QApplication([])
w = AppWindow()
w.show()
app.exec_()