x = 1/50 : 1/50 : 1;
y = (sin(2*pi*x) + 1)/2;

fprintf('{');
for i = 1: size(x,2)    
    fprintf('%5.0f, ',1023*y(i));
end

fprintf('}\n{');

for i = 1 : size(x,2)
    fprintf('%5.0f, ',1023*x(i));
end

fprintf('}\n{');

for i = 1 : size(x,2)
    if(i < 25)
        fprintf('%5.0f, ', 1023*x(i)*2);
    else
        fprintf('%5.0f, ', 1023*2-1023*x(i)*2);
    end
end