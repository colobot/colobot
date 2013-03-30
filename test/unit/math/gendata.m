% Script in Octave for generating test data

1;

% Returns the minor matrix
function m = minor(A, r, c)

  m = A;
  m(r,:) = [];
  m(:,c) = [];

end;

% Returns the cofactor matrix
function m = cofactors(A)

  m = zeros(rows(A), columns(A));

  for r = [1 : rows(A)]
    for c = [1 : columns(A)]
      m(r, c) = det(minor(A, r, c));
      if (mod(r + c, 2) == 1)
        m(r, c) = -m(r, c);
      end;
    end;
  end;

end;

% Prints the matrix as C++ code
function printout(A, name)

  printf('const float %s[16] = \n', name);
  printf('{\n');

  for c = [1 : columns(A)]
    for r = [1 : rows(A)]
      printf('  %f', A(r,c));
      if (! ( (r == 4) && (c == 4) ) )
        printf(',');
      end;
      printf('\n');
    end;
  end;

  printf('};\n');

end;

printf('// Cofactors\n');
A = randn(4,4);
printout(A, 'COF_MAT');
printf('\n');
printout(cofactors(A), 'COF_RESULT');
printf('\n');

printf('\n');

printf('// Det\n');
A = randn(4,4);
printout(A, 'DET_MAT');
printf('\n');
printf('const float DET_RESULT = %f;', det(A));
printf('\n');

printf('\n');

printf('// Invert\n');
A = randn(4,4);
printout(A, 'INV_MAT');
printf('\n');
printout(inv(A), 'COF_RESULT');
printf('\n');

printf('\n');

printf('// Multiplication\n');
A = randn(4,4);
printout(A, 'MUL_A');
printf('\n');
B = randn(4,4);
printout(B, 'MUL_B');
printf('\n');
C = A * B;
printout(C, 'MUL_RESULT');
printf('\n');
