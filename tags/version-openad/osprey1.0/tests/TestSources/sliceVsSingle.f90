subroutine foo(x)
  integer :: x
  x=x+1
end subroutine

program arrays3
  integer :: x(2)
  x(1)=1
! see if this is unparsed as 
! a slice rather than a scalar
  call foo(x(1))
  if (x(1).eq.2) then 
	print *, 'OK'
  else
	print *, 'failed'
  endif 
end program
