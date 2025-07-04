%function [exit_code, tInterval,Th1,Th2,Th3,Th4,Th5]=CI_Scorbot(p1,p2,limitare_viteza)
function [exit_code, tTimes,Th1,Th2,Th3,Th4,Th5,my_vel,my_acc,xpos,ypos,zpos]=CI_Scorbot(p1, p2, limitare_viteza)

exit_code=-1;

% Transfomarea punctelor într-un spațiu 3D cu o rotație de 180 de grade în
% jurul axei x
Tf_P1 = trvec2tform([p1(1) p1(2) p1(3)])*axang2tform([1 0 0 pi]);
Tf_P2 = trvec2tform([p2(1) p2(2) p2(3)])*axang2tform([1 0 0 pi]);
tform2eul(Tf_P1, 'XYZ');
tform2eul(Tf_P2, 'XYZ');

% Calculul timpului total pe baza vitezei maxime și a distanței euclidiene
% dintre cele două puncte între care se dorește deplasarea liniară
viteza=limitare_viteza;
distanta=sqrt((p1(1)-p2(1))^2+(p1(2)-p2(2))^2+(p1(3)-p2(3))^2);
t_total=distanta/viteza;

% Stabilirea numărului de puncte intermediare
N=ceil(distanta) * 2; % de modificat acest numar

% Generarea unui vector de timp
tTimes = linspace(0,t_total,N);
tInterval = [0 t_total];

% Generarea traiectoriei
[s,sd,sdd] = trapveltraj([0 1],numel(tTimes));

% Interpolarea traiectoriei 
[T,dT,ddT] = transformtraj(Tf_P1,Tf_P2,tInterval,tTimes,'TimeScaling',[s;sd;sdd]);

% se aplica cinematica inversa pentru intervalul P1-P2
for i=1:N
    my_orient_poz=tform2eul(T(:,:,i), 'XYZ');

    myT=T(:,:,i);
    translations(i,:) = myT(1:3,4)';  % Extract translation (position)
    rotations(i,:) = rotm2quat(myT(1:3,1:3)); % Convert rotation matrix to quaternion

    [my_error,Th1(i),Th2(i),Th3(i),Th4(i),Th5(i)] = my_CI_er7(T(1,4,i),T(2,4,i),T(3,4,i), -pi/2,0);

    xpos(i)=T(1,4,i);
    ypos(i)=T(2,4,i);
    zpos(i)=T(3,4,i);
end

my_vel=dT;
my_acc=ddT;
figure; hold on; grid on;
plotTransforms(translations, rotations, 'FrameSize', 10); % Adjust frame size
plotTransforms([0 0 0], [1 0 0 0], 'FrameSize', 1.5); % Black frame at (0,0,0)
plotTransforms([500 500 200], [1 0 0 0], 'FrameSize', 1.5); % Black frame at (0,0,0)
xlim([0,500]); ylim([0,500]); zlim([0,400]);
xlabel('X'); ylabel('Y'); zlabel('Z');
title('Deplasare liniară între două puncte');
axis equal; view(3);
hold off;