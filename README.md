# OS-preemptive-threading
1. 在terminal,利用make編譯testparking.c, preemptive.c
2. 在testparking.map中，可看到各個函數在編好的組合語言代碼中的代碼記憶體位址。
    可據此設立中斷點。並知曉代碼執行到什麼地方，資料記憶體應該做什麼相應的改變。
3. 在Edsim51中載入testparking.hex、assemble source code、run
