function [imageCha, ssim_map, metrics] = algo_FCSA_proxA_2D( obj,input )
% 2D variant of FCSA
% based on Huang et al. paper on FCSA
%
% input:
% obj           CS reconstruction object (holding all parameters)
% input         struct containing recon parameters and image
%
% output:
% imageCha      reconstructed channel individual image
% ssim_map      structural similarity map
% metrics       evaluation metrics 
%
% (c) Marc Fischer, Thomas Kuestner, May 2015
% -------------------------------------------------------------------------

%%
timer_proxA = tic;

%% variables:
% internal flags
flag_wavetree = true;
flag_fast = true;
flag_extendImage = true;

% internal variables:
L = 1;
t_old = 1;
NLTV_struct.kernelratio = 3;
NLTV_struct.windowratio = 6;
NLTV_struct.nThreads = 1;  % mind this option if used with -singleCompThread on BWCluster
itrNLTV = obj.iNINNER - 5;
% chambolle tv:
parsin.MAXITER=100; parsin.tv='iso'; % 'iso' or 'l1'

% from obj:
% maxitr = obj.maxitr;
maxitr = obj.iNINNER;
% n1 = obj.measPara.dim(1);
% n2 = obj.measPara.dim(2);
n1 = input.n1;
n2 = input.n2;
% nSlices = obj.measPara.dim(3);
nCha = obj.measPara.dim(5);
lambdaWave = obj.lambda;
lambdaTV = obj.lambdaTV;
lambdaGroup = obj.lambdaGroup;
NLTV_struct.filterstrength = obj.lambdaNLTV_h; % 0.03 % converted from NLTV h = 0.01 %old: used: 3e-10
lambdaNLTV = obj.lambdaNLTV;
lambdaNLTV_h = obj.lambdaNLTV_h;
regularizerWeights = obj.regularizerWeights;
flagTV = obj.flagTV;
flagTV_iso = obj.flagTV_iso;
flagWave = obj.flagWave;
flagGroup = obj.flagGroup;
flagNLTV = obj.flagNLTV;
flagSBNLTV = obj.flagSBNLTV;
flagRealAndImag = obj.flagRealAndImag;
waveletStages = obj.trafo.waveletStages;
waveletFilterName_l1 = obj.trafo.waveletFilterName_l1;
waveletFilterName_l12 = obj.trafo.waveletFilterName_l12;

% from input:
b=input.b;
mask = input.mask;
G_prox = input.G_prox;
Gt_prox = input.Gt_prox;
groupnorm_index = input.groupnorm_index;
waveS_l1 = input.waveS_l1;
waveS_l12 = input.waveS_l12;
waveS_l12proxA = input.waveS_l12proxA;
proxA_extend_y = waveS_l12proxA(waveletStages+2,1) - waveS_l12(waveletStages+2,1);
proxA_extend_x = waveS_l12proxA(waveletStages+2,2) - waveS_l12(waveletStages+2,2);
z_proxA = cell(1,nCha);

% im_ref = input.im_ref;
% im_ref_full = zeros(n1,n2);
% for j = 1:nCha
%     im_ref_full = im_ref_full + abs(im_ref{1,j}).^2;
% end;
% im_ref_full = sqrt(im_ref_full);
clear input

% initialize cells/vectors
for j=1:nCha
    FTb{1,j} = iFFT2D(b{1,j});
    y{1,j} = real(FTb{1,j}); % needed for y_old
    y{1,j+nCha} = imag(FTb{1,j});
end;
z = FTb; % starting point

x_wave = cell(1,2*nCha);
x_helper = x_wave;
x_wave_helper = x_wave;
x_tv = x_wave;
x_nltv = x_wave;
for j=1:2*nCha
    x_nltv{1,j} = 0;
end;
x_g = x_wave;
x_g_helper = x_wave;
x_g_proxA = x_wave;
z_comp = x_wave;

%% MAD dependent lambdas:
flag_MAD = true;
if flag_MAD
    x_wavedec = cell(1,nCha);
    threshold = zeros(1,nCha);
    if flagRealAndImag
        for j=1:nCha
            x_wavedec{1,j} = wavedec2(abs(z{1,j}),waveletStages,waveletFilterName_l1); % atm only based on l1-daubechie
            x_wave_fine_scale = size(x_wavedec{1,j},2) - (3*waveS_l1(waveletStages+1,1)*waveS_l1(waveletStages+1,2));
            threshold(j) = mad(x_wavedec{1,j}(x_wave_fine_scale:end),1);
        end;
    else
        for j=1:nCha
            x_wavedec{1,j} = wavedec2(real(z{1,j}),waveletStages,waveletFilterName_l1); % atm only based on l1-daubechie
            x_wavedec{1,j+nCha} = wavedec2(imag(z{1,j}),waveletStages,waveletFilterName_l1); % atm only based on l1-daubechie
            x_wave_fine_scale_real = size(x_wavedec{1,j},2) - (3*waveS_l1(waveletStages+1,1)*waveS_l1(waveletStages+1,2));
            threshold(j) = mad(x_wavedec{1,j}(x_wave_fine_scale_real:end),1);
            x_wave_fine_scale_imag = size(x_wavedec{1,j},2) - (3*waveS_l1(waveletStages+1,1)*waveS_l1(waveletStages+1,2));
            threshold(j+nCha) = mad(x_wavedec{1,j}(x_wave_fine_scale_imag:end),1);
        end;
    end;
    clear x_wavedec
else
    threshold(1:nCha) = 1;
end;

if flagRealAndImag
    threshold_group = 0;
    for j = 1:nCha
        threshold_wave(j) = lambdaWave * threshold(j) * 2/L;
        threshold_TV(j) = lambdaTV * threshold(j) * 2/L;
        threshold_TV(j+nCha) = lambdaTV * threshold(j) * 2/L;        
        threshold_group = threshold_group + lambdaGroup * threshold(j) * 2/L;
        threshold_NLTV(j) = lambdaNLTV; % * threshold(j) * 2/L;
        threshold_NLTV(j+nCha) = lambdaNLTV;
        threshold_NLTV_h(j) = lambdaNLTV_h; %*threshold(j); % adjust carefully or NLTV won't find a solution. lambdaNLTV_h should be < 0.01
        threshold_NLTV_h(j+nCha) = lambdaNLTV_h;
    end;
    threshold_group = threshold_group /3;
else
    threshold_group_real = 0;
    threshold_group_imag = 0;
    for j = 1:nCha
        threshold_wave(j) = lambdaWave * threshold(j) * 2/L;
        threshold_wave(j+nCha) = lambdaWave * threshold(j+nCha) * 2/L;
        threshold_TV(j) = lambdaTV * threshold(j) * 2/L;
        threshold_TV(j+nCha) = lambdaTV * threshold(j+nCha) * 2/L;
        threshold_group_real = threshold_group_real + lambdaGroup * threshold(j) * 2/L;
        threshold_group_imag = threshold_group_imag + lambdaGroup * threshold(j) * 2/L;
        threshold_NLTV(j) = lambdaNLTV; % * threshold(j) * 2/L;
        threshold_NLTV(j+nCha) = lambdaNLTV;
        threshold_NLTV_h(j) = lambdaNLTV_h; %*threshold(j); % adjust carefully or NLTV won't find a solution. lambdaNLTV_h should be < 0.01
        threshold_NLTV_h(j+nCha) = lambdaNLTV_h;
    end;
    threshold_group_real = threshold_group_real /3;
    threshold_group_imag = threshold_group_imag /3;
end;

%% initialize metrics:
itr = 0;
metrics.xtime(itr+1)= 0;
% [metrics, ssim_map{1,1}] = get_metrics_itr( im_ref, im_ref_full, z, itr, maxitr, nCha, n1, n2, metrics, obj.K_1, obj.K_2, obj.W_size, obj.W_sigma );
ssim_map = [];

%% recon

dispProgress('Proximal Average', 0, maxitr);
for itr = 1:maxitr  % total iter counter
    
    t_new = (1+sqrt(1+4*t_old^2))/2;
    t_old = t_new;
    
    y_old = y;   % y_old = y for complex case
    
    %% landweber step
    for j = 1:nCha
        x_helper{1,j} = iFFT2D(FFT2D_mask(z{1,j},mask)) -FTb{1,j} ;
        z{1,j} = z{1,j} - x_helper{1,j}/L;
        z_comp{1,j} = real(z{1,j});
        z_comp{1,j+nCha} = imag(z{1,j});
    end;
    
    %% l1-Wavelet
    if flagWave
        for j = 1:2*nCha
            x_wave_helper{1,j} = wavedec2(z_comp{1,j},waveletStages,waveletFilterName_l1);
        end;
        if flagRealAndImag
            for j = 1:nCha
                [x_wave_helper{1,j}, x_wave_helper{1,j+nCha}] = groupthresh_2vec(x_wave_helper{1,j},x_wave_helper{1,j+nCha},threshold_wave(j));
            end;
        else
            for j = 1:2*nCha
                x_wave_helper{1,j} = softthresh_real(x_wave_helper{1,j},threshold_wave(j));
            end;
        end;
        for j = 1:2*nCha
            x_wave{1,j} = waverec2(x_wave_helper{1,j},waveS_l1,waveletFilterName_l1);
        end;
    end;
    
    %% TV
    if flagTV
        if ~flagTV_iso
            for j = 1:2*nCha
                x_tv{1,j} = MTV_2D(z_comp{1,j},threshold_TV(j),n1,n2);
            end;
        else
            for j = 1:2*nCha
                if (itr==1)
                    [x_tv{1,j}, P]=denoise_TV_One((z_comp{1,j}), threshold_TV(j),-inf,inf,[],parsin);
                else
                    [x_tv{1,j}, P]=denoise_TV_One((z_comp{1,j}), threshold_TV(j),-inf,inf,P,parsin);
                end;
            end;
        end;
    end;
    
    %% NLTV
    if flagNLTV
        if itr >= itrNLTV
            if flagSBNLTV
                for j = 1:2*nCha
                    x_nltv{1,j} = SB_NLTVfunc_slim_rescale(z_comp{1,j},n1,n2, threshold_NLTV(j), threshold_NLTV_h(j) );
                end;
            else
                for j = 1:2*nCha
                    % if mod(itr,5) == 0 || itr == 1
                    x_nltv{1,j} = NLMF(z_comp{1,j},NLTV_struct);
                    x_nltv{1,j} = (L.*z_comp{1,j} + 2*threshold_NLTV(j)*x_nltv{1,j})./(L+2*threshold_NLTV(j));
                    % end;
                end;
            end;
        end;
    end;
    
    %% l12-Wavelet
    if flagGroup
        for j = 1:2*nCha
            if flag_extendImage
                z_proxA{1,j} = extend_image(z_comp{1,j}, waveS_l12proxA, waveletStages, proxA_extend_y, proxA_extend_x);
                x_g_helper{1,j} = wavedec2(z_proxA{1,j},waveletStages,waveletFilterName_l12);
            else
                x_g_helper{1,j} = wavedec2(z_comp{1,j},waveletStages,waveletFilterName_l12);
            end;
            
            if flag_wavetree
                x_g_helper{2,j} = (G_prox*x_g_helper{1,j}')';
            else
                x_g_helper{2,j} = zeros(1,size(x_g_helper{1,j},2));
            end;
        end;
        if flagRealAndImag
            x_g_helper(1,:) = softthresh_proxA_cha(x_g_helper,threshold_group,2*nCha,waveS_l12proxA,groupnorm_index);
        else
            x_g_helper(1,1:nCha) = softthresh_proxA_cha(x_g_helper(:,1:nCha),threshold_group_real,nCha,waveS_l12proxA,groupnorm_index);
            x_g_helper(1,nCha+1:2*nCha) = softthresh_proxA_cha(x_g_helper(:,1:nCha),threshold_group_imag,nCha,waveS_l12proxA,groupnorm_index);
        end;
        for j = 1:2*nCha
            x_g_proxA{1,j} = waverec2(x_g_helper{1,j},waveS_l12proxA,waveletFilterName_l12);
            x_g{1,j} =  x_g_proxA{1,j}(1:end-proxA_extend_y,1:end-proxA_extend_x);
        end;
    end;
    
    %% add prox(.)
    for j = 1:2*nCha
        y{1,j} = zeros(n1,n2);
        if flagWave y{1,j} = y{1,j} + x_wave{1,j}.*regularizerWeights(1); end;
        if flagTV y{1,j} = y{1,j} + x_tv{1,j}.*regularizerWeights(2); end;
        if flagGroup y{1,j} = y{1,j} + x_g{1,j}.*regularizerWeights(3); end;
        if flagNLTV y{1,j} = y{1,j} + x_nltv{1,j}.*regularizerWeights(4); end;
        
        if itr < itrNLTV
            y{1,j} = y{1,j}/(flagTV.*regularizerWeights(2) + flagWave.*regularizerWeights(1) + flagGroup.*regularizerWeights(3));
        else
            y{1,j} = y{1,j}/(flagTV.*regularizerWeights(2) + flagNLTV.*regularizerWeights(4) + flagWave.*regularizerWeights(1) + flagGroup.*regularizerWeights(3));
        end;
        
        if flag_fast
            y{1,j}=y{1,j}+((t_old-1)/t_new).*(y{1,j}-y_old{1,j});
        end;
    end;
    
    %% metrics of current itr:
%     disp(itr);
    dispProgress('Proximal Average', itr/maxitr);
    
    metrics.xtime(itr+1)= toc(timer_proxA);
    for j = 1:nCha
        z{1,j} = y{1,j} + 1i*y{1,j+nCha};
    end;
%     [metrics, ssim_map{1,2}] = get_metrics_itr( im_ref, im_ref_full, z, itr, maxitr, nCha, n1, n2, metrics, obj.K_1, obj.K_2, obj.W_size, obj.W_sigma );
    
end;
dispProgress('Proximal Average', 'Close');

imageCha = z;
for j = 1:nCha
    imageCha{1,j} = turn_image( imageCha{1,j} );
end;
% for j = 1:nCha+1
%     ssim_map{1,1}{1,j} = turn_image( ssim_map{1,1}{1,j} );
%     ssim_map{1,2}{1,j} = turn_image( ssim_map{1,2}{1,j} );
% end;

end

