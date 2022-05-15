%解猫置换图像
%Arnold Transformation
%2022/5/14 by linyiting
%https://gitee.com/DoNotWantToGrowUp/matlab-study/
function out= DeArnold(image,sque)
    [H,W] = size(image);
    d = H*W;
    image = double(image);
    x1 = reshape(sque(1:d),H,W);
    x2=reshape(sque(d+1:2*d),H,W);
    imgTmp = image;
    for i = H : -1 : 1
        for j = W : -1 : 1
            k = mod([1 x1(i,j);x2(i,j) x1(i,j)*x2(i,j)+1]*[i;j],[H;W])+[1;1];
            t = imgTmp(i,j);
            imgTmp(i,j) = imgTmp(k(1),k(2));
            imgTmp(k(1),k(2)) = t;
        end
    end
    out = imgTmp;
end