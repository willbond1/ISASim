#from . import assembler
#from . import cpu
#from . import isasim
#from . import mem

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

        def gen(max):
            num = 0
            while num<max:
                yield str(num)
                num = num+1

        self.ui.memList_1.setVerticalHeaderLabels(gen(1000000))
        self.ui.memList_2.setVerticalHeaderLabels(gen(1000000))

        self.ui.memList_1.setRowCount(0);
        #self.ui.memList_1.insertRow(1);

        def loadtable1():
            print(self.ui.memCombo_1.currentText())
        def loadtable2():
            print(self.ui.memCombo_2.currentText())
        self.ui.memCombo_1.currentIndexChanged.connect(loadtable1)
        self.ui.memCombo_2.currentIndexChanged.connect(loadtable2)

        def setcacheonoff():
            print(self.ui.cacheOn.isChecked())
        self.ui.cacheOn.stateChanged.connect(setcacheonoff)
        def setpipelineonoff():
            print(self.ui.pipelineOn.isChecked())
        self.ui.pipelineOn.stateChanged.connect(setpipelineonoff)

        self.show()

app = QApplication([])
w = AppWindow()
w.show()
app.exec_()