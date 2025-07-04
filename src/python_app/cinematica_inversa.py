import matlab.engine

eng = matlab.engine.start_matlab()
eng.cd(r"D:/UPB/Licenta/MatlabV2", nargout=0)
eng.workspace['p1'] = matlab.double([350, 100, 100]) #[350, 100, 100]
eng.workspace['p2'] = matlab.double([350, 100, 200])
eng.workspace['limitare_viteza'] = matlab.double(40)

eng.run("main", nargout=0)

rezultat = eng.workspace['sirValori']
eng.quit()

print(rezultat)
