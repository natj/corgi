import unittest

import sys
sys.path.append('pycorgi')
sys.path.append('tests')


import example


class Initialization(unittest.TestCase):

    i = 10
    j = 11
    o = 1

    Nx = 10
    Ny = 20

    def setUp(self):
        self.cell1 = example.Welsh(self.i, self.j, self.o, self.Nx, self.Ny)
        self.cell2 = example.Pembroke(self.i, self.j, self.o, self.Nx, self.Ny)

    #test that derived classes can inherit base class methods
    def test_inheritance(self):

        (i,j) = self.cell1.index()
        self.assertEqual(i, self.i)
        self.assertEqual(j, self.j)

        (i,j) = self.cell2.index()
        self.assertEqual(i, self.i)
        self.assertEqual(j, self.j)

    #tests that dserived classes can be extended
    def test_extending(self):

        self.assertEqual( self.cell1.bark(), "Woof!" )
        self.assertEqual( self.cell2.bark(), "Ruff!" )

        self.assertEqual( self.cell2.howl(), "Auuuuuu!" )


def cellID(i,j,Nx,Ny):
    return j*Nx + i


class ParallelGrid(unittest.TestCase):
    
    Nx = 10
    Ny = 15

    xmin = 0.0
    xmax = 1.0
    ymin = 2.0
    ymax = 3.0


    def setUp(self):
        self.node = example.Grid(self.Nx, self.Ny)
        self.node.setGridLims(self.xmin, self.xmax, self.ymin, self.ymax)


    def test_extension(self):
        self.assertEqual( self.node.petShop(), "No Corgis for sale.")


    def test_cid(self):
        for j in range(self.node.getNy()):
            for i in range(self.node.getNx()):
                cid = self.node.cellId(i, j)
                cidr = cellID( i, j, self.node.getNx(), self.node.getNy() )
                self.assertEqual(cid, cidr)

    def mpiInitialization(self):

        self.node.initMpi()

        self.refGrid = np.zeros((self.Nx, self.Ny), np.int)
        self.refGrid[0:5,   0:10] = 0
        self.refGrid[0:5,  10:15] = 1
        self.refGrid[5:10,  0:10] = 2
        self.refGrid[5:10, 10:15] = 3

        if self.node.master:
            for j in range(self.node.getNy()):
                for i in range(self.node.getNx()):
                    val = self.refGrid[i,j]
                    self.node.setMpiGrid(i, j, val )
        self.node.bcastMpiGrid()

        for j in range(self.node.getNy()):
            for i in range(self.node.getNx()):
                val = self.node.mpiGrid(i,j)
                self.assertEqual(val, self.refGrid[i,j])
        self.node.finalizeMpi()



    def test_loading(self):

        k = 0
        for j in range(self.node.getNy()):
            for i in range(self.node.getNx()):
                c = example.Welsh(i, j, 0, self.node.getNx(), self.node.getNy() )
                self.node.addCell(c) 
                k += 1
        self.assertEqual( k, self.Nx*self.Ny )
        self.assertEqual( len(self.node.getCellIds()), self.Nx*self.Ny )
        #print self.node.getCells()


         


if __name__ == '__main__':
    unittest.main()

