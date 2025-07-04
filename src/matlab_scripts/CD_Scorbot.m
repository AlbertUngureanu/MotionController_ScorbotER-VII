function [px, py, pz] = CD_Scorbot(Th1, Th2, Th3, Th4, Th5)

% Parametrii robotului
a1 = 50;
a2 = 300;
a3 = 350;
d1 = 358.5;
d5_tool = 251;
alfa4 = -pi/2;
alfa5 = 0;

% 1. Coordonate în plan vertical (PR și PZ fără tool offset)
var_PR = a2 * cos(Th2) + a3 * cos(Th2 - Th3);
var_PZ = a2 * sin(Th2) + a3 * sin(Th2 - Th3);

% 2. Tool offset (poziție efectivă a punctului condus)
r = a1 + var_PR + d5_tool * cos(alfa4);
pz = d1 + var_PZ + d5_tool * sin(alfa4);

% 3. Conversie din polar la cartezian
px = r * sin(Th1);
py = r * cos(Th1);
