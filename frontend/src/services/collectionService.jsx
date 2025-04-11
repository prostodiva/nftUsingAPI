import axios from 'axios';
import { API_URL } from '../config/config';

export const collectionService = {
    getAllCollections: async () => {
        try {
            console.log('Fetching collections from:', `${API_URL}/api/collections`);
            const response = await axios.get(`${API_URL}/api/collections`);
            console.log('Collections response:', response.data);
            
            if (response.data.status === 'success') {
                return response.data;
            } else {
                throw new Error(response.data.message || 'Failed to fetch collections');
            }
        } catch (error) {
            console.error('API error:', {
                message: error.message,
                response: error.response?.data,
                status: error.response?.status,
                url: error.config?.url
            });
            throw new Error(error.response?.data?.message || 'Failed to fetch collections');
        }
    }
};