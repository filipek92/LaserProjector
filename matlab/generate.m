% This script need array called 'image'
imsize = [480,400];
file = '..\include\img.h';

assert(mod(imsize(1), 8)==0)

if ~exist('image', 'var')
    load('image')
end

img = imresize(image, imsize)>127;
imshow(img)
clear data
%%
data = zeros(1, size(img,2)*size(img,1)/8, 'uint8');
for indx = 1:size(img,2)
    line = img(end:-1:1, indx);
    for indy = 1:8:length(line)
        part = line(indy:indy+7);
        ind = (indx-1)*size(img,1)/8 + ((indy-1)/8) +1;
        data(ind) = bin2dec(char(part'+'0'));
    end
end

%% export to c file
f = fopen(file, 'w');

fprintf(f, '#define X_SIZE %d\n', imsize(2));
fprintf(f, '#define Y_SIZE %d\n', imsize(1));

fprintf(f, 'uint8_t img[IMG_BUFFER] = {', imsize(2)*imsize(1)/8);
fprintf(f, '%d,', data);
fwrite(f, '};');

fclose(f);
%%
open(file)

%%
s = serial('COM5', 'Baudrate', 921600);
s.Terminator = 'CR/LF';
fopen(s);
fwrite(s, 17);

%%
tic
fwrite(s, 17);
while s.BytesAvailable
    fread(s, s.BytesAvailable); 
end
fprintf('\nStarting transfer\n');
command = sprintf('transfer %d\n', length(data));%length(data));
fwrite(s, command);
pause(0.05);
while s.BytesAvailable
    rep = char(fread(s, s.BytesAvailable)');
    disp(rep(1:end-2));
end
for i = 1:100:length(data)
    fwrite(s, data(i:i+99));
end
pause(0.1);
while s.BytesAvailable
    rep = char(fread(s, s.BytesAvailable)');
    disp(rep(rep>20));
end
toc
fprintf('Done\n');
cmd = sprintf('resolution %d %d\n', imsize(2), imsize(1));
fwrite(s, cmd);
pause(0.1);
while s.BytesAvailable
    rep = char(fread(s, s.BytesAvailable)');
    disp(rep(rep>20));
end
%%
devs = instrfindall;
if ~isempty(devs)
    fclose(instrfindall);
end


