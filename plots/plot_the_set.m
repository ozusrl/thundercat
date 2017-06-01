%USAGE: 
%>> plot_the_set('small_set')
%>> plot_the_set('large_set')

function plot_the_set(matrix_set)
m_set = strcat (matrix_set,'.mat');
load(m_set);
fig=figure;
 x=1:size(names,1);
 hold on
 scatter (csrbynzspeedup, x', 'red', 'filled');
 scatter (stencilspeedup, x', 'blue', 'filled')
 scatter (genoski33speedup, x',   'black', 'filled')
 scatter (genoski44speedup, x', 'magenta', 'filled')
 scatter (genoski55speedup, x', 'black', 'filled','s')
 
 xtick=x;
 
%  legend("CSRbyNZ", "Stencil", "GenOSKI33", "GenOSKI44", "GenOSKI55"); % works fine on MATLAB. problematic on octave
 
 textdata = names;
 label = char(textdata);
 set(gca,'ytick',xtick);
 ytick = 0:0.2:3;
 set(gca,'xtick',ytick);
 set(gca,'yticklabel',strtrim(label));
 
 set(gca,'fontsize',12);
 set(gca,'layer','top');
 
 