%aceasta functie face cinematica inversa pentru robotul Scorbot ER7 pe baza
%pozitiei punctului condus
function [exit_code, Th1,Th2,Th3,Th4,Th5]=my_CI_er7(px,py,pz, alfa4, alfa5)

%dimensiuni segmente robot conform documentatie docArticulatii.pptx
% % Define SCORBOT-ER VII link lengths (in mm)
a1 = 50; % Link 1 length
a2 = 300; % Link 2 length
a3 = 350; % Link 2 length
d1 = 358.5; % Base height
d5_tool = 251;  % control point offset relative to joint 4 (+Wrist offset)

%initializare cod eroare la -1 (tot este bine) si valori unghiuri la 0
exit_code=-1;
Th1=0;
Th2=0;
Th3=0;
Th4=0;
Th5=0;

%verificare punct in spatiul de lucru
if(sqrt(px^2+py^2)>950)
    %cod eroare 1 punct in afara anvelopei de lucru (>700mm)
    disp("punct in afara spatiului de lucru");
    exit_code=1;
    return
end

%calcul Theta 1
Th1=atan2(px,py);

%calcul dimensiuni triunghi segment 2&3
var_PZ=pz-d1-d5_tool*sin(alfa4);
var_PR=sqrt(px^2+py^2)-a1-d5_tool*cos(alfa4);
sum_2_PR_PZ=var_PR^2+var_PZ^2;

%calcul Theta 3 (Theta 2 se calculeaza in functie de acesta
Th3=pi-acos((a2^2+a3^2-var_PR^2-var_PZ^2)/(2*a2*a3));

%calcul Theta 2
Th2=atan2(var_PZ,var_PR)+acos((a2^2+sum_2_PR_PZ-a3^2)/(2*a2*sqrt(sum_2_PR_PZ)));

%calcul Theta 4
Th4=abs(alfa4)+Th2-Th3;

%calcul Theta 5
Th5=alfa5;
