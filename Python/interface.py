from . import assembler
from . import cpu
from . import isasim
from . import mem

from PyQt5.QtWidgets import *

app = QApplication([])
button = QPushButton('Test')
def on_button_clicked():
    alert = QMessageBox()
    alert.setText('You clicked the button!')
    alert.exec_()

button.clicked.connect(on_button_clicked)
button.show()
app.exec_()

#if __name__ == "__main__":