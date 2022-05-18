function out = abs( in )
% ABS for nested cells
%
% (c) Thomas Kuestner 
% ---------------------------------------------------------------------

[out, idx] = flattenCellMatrix(in.data);
out = cellfun(@(x) abs(x), out, 'UniformOutput', false);
out = reconFlatCellMatrix(out,idx);
out = TRAFO(out,in.meta);
% if(nargout > 0)
%     out = TRAFO(out); % return TRAFO object again without modifying input
% else
%     in.data = out;
% end

end

