%zigzag置换图像
%ZigzagTransformation
%2022/5/14 revise by linyiting
%https://gitee.com/DoNotWantToGrowUp/matlab-study/
function B = Zigzag(A)
% zigzag正变换
%注意：只适用于行列相等的图。
[r,c] = size(A);
if r ~= c
    error('input array should have equal number of rows and columns')
end
N = r;
len = N*N;
B = zeros(1,len);                                                     % zigzag排序后的数组
B(1) = A(1);
k = 1; 
i = 1; j = 1;                                                              % 起始坐标
d = 1;        % 排序的方向标示。值为1表示下一个元素在右上方向；值为-1表示下一个元素在左下方向
flag = 1;     % 上下三角标示。值为1表示当前元素在上三角；值为0表示当前元素在下三角
while k < len
       if flag == 1                                % 当前元素为上三角
        if i ~= 1 && j ~= 1                                                % 非边界元素
            if d == 1                              % 下一个元素在右上方向
                i = i-1; j = j+1;
            else                                   % 下一个元素在左下方向
                i = i+1; j = j-1;
            end
        else
            if i == 1                                                      % 上边界                
                if d == 1                                                  % 需要变向
                    j = j+1; d = -1;
                else                                                       % 不需要变向
                    i = i+1; j = j-1;
                end
                
            else
                if j == 1                                                  % 左边界
                    
                    if d == 1                                              % 不需要变向
                        i = i-1; j = j+1;
                    else                                                   % 需要变向
                        i = i+1; d = 1;
                    end               
                end
            end
        end
       else                                                % 当前元素为下三角的时候
        if i ~= N && j ~= N
            if d == 1
                i = i-1; j = j+1;
            else
                i = i+1; j = j-1;
            end
        else
            if i == N
                if d == 1
                    i = i-1; j = j+1;
                else
                    j = j+1; d = 1;
                end
            else
                if j == N
                    if d == 1
                        i = i+1; d = -1;
                    else
                        i = i+1; j = j-1;
                    end
                end
            end
        end
       end
    if j == N         % 若元素到达了右边界，说明上三角元素已经遍历完，开始遍历下三角元素
        flag = 0;
    end    
    k = k+1;
    B(k) = A(i,j);
end