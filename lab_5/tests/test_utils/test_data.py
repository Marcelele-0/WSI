import unittest
import numpy as np
import sys
import os

# Add src to path to import modules
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'src'))

from utils.data import generate_data, normalize_data


class TestGenerateData(unittest.TestCase):
    """Test cases for generate_data function."""
    
    def test_generate_data_valid_input(self):
        """Test generate_data with valid input."""
        n_samples = 100
        X, y = generate_data(n_samples)
        
        # Check shapes
        self.assertEqual(X.shape, (n_samples, 2))
        self.assertEqual(y.shape, (n_samples, 1))
        
        # Check data types
        self.assertEqual(X.dtype, np.float32)
        self.assertEqual(y.dtype, np.float32)
        
        # Check value ranges
        self.assertTrue(np.all(X >= -1))
        self.assertTrue(np.all(X <= 1))
        self.assertTrue(np.all(np.isin(y, [0.0, 1.0])))
    
    def test_generate_data_invalid_input(self):
        """Test generate_data with invalid input."""
        with self.assertRaises(ValueError):
            generate_data(-1)
        with self.assertRaises(ValueError):
            generate_data(0)
    
    def test_generate_data_labels_logic(self):
        """Test that labels are generated correctly based on sign logic."""
        np.random.seed(42)
        X, y = generate_data(100)
        
        # Manually compute expected labels
        expected_y = (np.sign(X[:, 0]) == np.sign(X[:, 1])).astype(np.float32).reshape(-1, 1)
        
        # Check if labels match expected logic
        np.testing.assert_array_equal(y, expected_y)


class TestNormalizeData(unittest.TestCase):
    """Test cases for normalize_data function."""
    
    def setUp(self):
        """Set up test fixtures."""
        self.test_data = np.array([[3.0, 4.0], [1.0, 2.0], [0.0, 0.0]], dtype=np.float32)
        self.test_data_nonzero = np.array([[3.0, 4.0], [1.0, 2.0]], dtype=np.float32)
    
    def test_normalize_data_l1(self):
        """Test L1 normalization."""
        result = normalize_data(self.test_data_nonzero, norm_type='l1')
        
        # Check that L1 norm of each row is 1
        l1_norms = np.sum(np.abs(result), axis=1)
        np.testing.assert_array_almost_equal(l1_norms, [1.0, 1.0], decimal=5)
        
        # Manual calculation verification
        expected = self.test_data_nonzero / np.sum(np.abs(self.test_data_nonzero), axis=1, keepdims=True)
        np.testing.assert_array_almost_equal(result, expected)
    
    def test_normalize_data_l2(self):
        """Test L2 normalization."""
        result = normalize_data(self.test_data_nonzero, norm_type='l2')
        
        # Check that L2 norm of each row is 1
        l2_norms = np.sqrt(np.sum(result**2, axis=1))
        np.testing.assert_array_almost_equal(l2_norms, [1.0, 1.0], decimal=5)
        
        # Manual calculation verification
        expected = self.test_data_nonzero / np.sqrt(np.sum(self.test_data_nonzero**2, axis=1, keepdims=True))
        np.testing.assert_array_almost_equal(result, expected)
    
    def test_normalize_data_none(self):
        """Test no normalization."""
        result = normalize_data(self.test_data, norm_type=None)
        np.testing.assert_array_equal(result, self.test_data)
        
        result = normalize_data(self.test_data, norm_type='none')
        np.testing.assert_array_equal(result, self.test_data)
    
    def test_normalize_data_zero_handling(self):
        """Test normalization with zero vectors."""
        zero_data = np.array([[0.0, 0.0], [3.0, 4.0]], dtype=np.float32)
        
        # L1 normalization should handle zero vectors
        result_l1 = normalize_data(zero_data, norm_type='l1')
        self.assertEqual(result_l1.shape, zero_data.shape)
        np.testing.assert_array_equal(result_l1[0], [0.0, 0.0])  # Zero vector stays zero
        
        # L2 normalization should handle zero vectors
        result_l2 = normalize_data(zero_data, norm_type='l2')
        self.assertEqual(result_l2.shape, zero_data.shape)
        np.testing.assert_array_equal(result_l2[0], [0.0, 0.0])  # Zero vector stays zero
    
    def test_normalize_data_invalid_type(self):
        """Test normalize_data with invalid normalization type."""
        with self.assertRaises(ValueError):
            normalize_data(self.test_data, norm_type='invalid')


if __name__ == '__main__':
    import logging
    logging.basicConfig(level=logging.WARNING)  # Reduce log noise during tests
    
    unittest.main()
