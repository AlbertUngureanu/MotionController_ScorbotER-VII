close all;
% p1, p2 si limitare_viteza vin din scriptul python
[exit_code, tTimes,Th1,Th2,Th3,Th4,Th5,my_vel,my_acc,xpos,ypos,zpos] = CI_Scorbot(p1, p2, limitare_viteza);

% s-a gasit o solutie pentru a deplasa punctul condus de la p1 la p2
if exit_code == -1
    l = length(Th1);
    sirValori = strings(1, l);
    for i = 1 : 1 : l
        pozitiiArticulatii = [Th1(i) * (180 / pi) * 256, (Th2(i) * (180 / pi) - 90) * 260, Th3(i) * (180 / pi) * 260, Th4(i) * (180 / pi) * 166.66, Th5(i) * (180 / pi) * 256];
        pozitiiArticulatii = fix(real(pozitiiArticulatii));
        sirValori(i) = join(string(pozitiiArticulatii), ',');
    end
end
