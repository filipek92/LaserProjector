% This script need array called 'image'
imsize = [80,80];
file = '..\include\img.h';

assert(mod(imsize(1), 8)==0)

if ~exist('image', 'var')
    load('image')
end

img = imresize(image, imsize)>127;
imshow(img)


data = zeros(1, size(img,2)*size(img,1)/8, 'uint8');
%%
for indx = 1:size(img,2)
    line = img(:, indx);
    for indy = 1:8:length(line)
        part = line(indy:indy+7);
        part = part .* [128; 64; 32; 16; 8; 4; 2; 1];
        part = sum(part);
        ind = (indx-1)*size(img,1)/8 + ((indy-1)/8) +1;
        data(ind) = sum(part);
    end
end

%% export to c file
f = fopen(file, 'w');

fprintf(f, '#define X_SIZE %d\n', imsize(2));
fprintf(f, '#define Y_SIZE %d\n', imsize(1));
fprintf(f, '#define Y_SIZEB (Y_SIZE/8)\n\n');

fprintf(f, 'const uint8_t img[%d] = {', imsize(2)*imsize(1)/8);
fprintf(f, '%d,', data);
fwrite(f, '};');

fclose(f);
%%
open(file)