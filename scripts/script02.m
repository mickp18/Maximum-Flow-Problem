% test the max flow with DAG generated randomly

clear all
close all
clc

%% open file
T = readmatrix("edges.txt");

s = T(:, 1);
t = T(:, 2);
w = T(:, 3);

G = digraph(s, t, w);

[mf, GF] =  maxflow(G,1,1000,'augmentpath');
mf
% G.Nodes.nodeIDs
% s1 = string(s);
% t1 = string(t);
% allNodes = unique([s1; t1]);  % Get unique node names
% G.Nodes.Name = allNodes;
% 
% figure
% % h = plot(G,'Layout','layered');
% h = plot(G, ...
%     'EdgeLabel', G.Edges.Weight, ...
%     'EdgeFontSize', 4, ...
%     'NodeLabel', G.Nodes.Name, ...
%     'NodeColor', 'r', ...
%     'NodeFontSize', 6, ...
%     'Marker', 'o',...
%     'ArrowSize', 6 ...
% );
% 
% layout(h,'layered','Direction','right','Sources',[1], 'Sinks', [500], 'AssignLayers', 'asap' )



