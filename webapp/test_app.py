import pytest
import os
import sys
from unittest.mock import patch, MagicMock

# Add webapp directory to sys.path
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from app import app as flask_app
from fresca_data import fresca_fetch

@pytest.fixture
def client():
    flask_app.config['TESTING'] = True
    with flask_app.test_client() as client:
        yield client

def test_index_route(client):
    """Test the home page route."""
    with patch('app.is_fresca_link_running', return_value=False):
        response = client.get('/')
        assert response.status_code == 200
        assert b'FRESCA' in response.data

def test_chart_route(client):
    """Test the chart page route."""
    with patch('app.is_fresca_link_running', return_value=True):
        response = client.get('/chart')
        assert response.status_code == 200
        assert b'Chart' in response.data

def test_fresca_running_api(client):
    """Test the /fresca_running status API."""
    with patch('app.is_fresca_link_running', return_value=True):
        response = client.post('/fresca_running')
        assert response.status_code == 200
        assert response.json is True

def test_fresca_fetch_logic_missing_file():
    """Test fresca_fetch with a non-existent file."""
    # Ensure a file that definitely doesn't exist
    result = fresca_fetch('non_existent_log', 0)
    # The current implementation returns False if file not found
    assert result is False

@patch('psutil.process_iter')
def test_is_fresca_link_running(mock_proc_iter):
    """Test the process checking logic."""
    from app import is_fresca_link_running
    
    # Mock a running process
    mock_proc = MagicMock()
    mock_proc.info = {'cmdline': ['python', 'fresca_uart_link.py']}
    mock_proc_iter.return_value = [mock_proc]
    
    assert is_fresca_link_running() is True
    
    # Mock no running process
    mock_proc_iter.return_value = []
    assert is_fresca_link_running() is False
