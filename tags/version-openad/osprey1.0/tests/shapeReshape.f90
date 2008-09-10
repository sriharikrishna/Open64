module bla
end module

subroutine head(x,y)
 double precision, dimension(:,:) :: x
 double precision, dimension(10) :: y
 y = reshape(x,shape(y))
 write(*,*) 'OK'
end subroutine

program main
 double precision, dimension(2,5) :: x
 double precision, dimension(10) :: y
 call head(x,y)
end program
