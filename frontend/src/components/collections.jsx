import { useEffect, useState } from "react";
import { collectionService } from "../services/collectionService.jsx";
import { Card, CardContent, CardHeader, CardTitle } from "./ui/card.jsx";

const Collections = () => {
    const [collections, setCollections] = useState([]);
    const [loading, setLoading] = useState(false);
    const [error, setError] = useState(null);

    const fetchCollections = async () => {
        try {
            setLoading(true);
            setError(null);
            console.log('Starting to fetch collections...');
            const data = await collectionService.getAllCollections();
            console.log('Received collections data:', data);
            
            if (data.status === 'success' && Array.isArray(data.collections)) {
                setCollections(data.collections);
            } else {
                throw new Error('Invalid response format from server');
            }
        } catch (error) {
            console.error('Error in fetchCollections:', error);
            setError(error.message || 'Failed to load collections');
        } finally {
            setLoading(false);
        }
    };

    useEffect(() => {
        fetchCollections();
    }, []);

    return (
        <section>
            {loading && (
                <div className="text-center py-12">
                    <div className="inline-block h-8 w-8 animate-spin rounded-full border-4 border-solid border-current border-r-transparent"></div>
                    <p className="mt-4">Loading collections</p>
                </div>
            )}
            {error && (
                <div className="text-center py-12 text-red-500">
                    <p>Error: {error}</p>
                </div>
            )}

            {!loading && !error && (
                <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-8 max-w-6xl mx-auto">
                    {collections.length > 0 ? (
                        collections.map((collection) => (
                            <Card key={collection} className="overflow-hidden">
                                <CardHeader>
                                    <CardTitle>{collection}</CardTitle>
                                </CardHeader>
                                <CardContent>
                                    <div className="space-y-2">
                                        <p><strong>Name</strong> {collection}</p>
                                    </div>
                                </CardContent>
                            </Card>
                        ))
                    ) : (
                        <div className="col-span-3 text-center py-12">
                            <p>No collections found.</p>
                        </div>
                    )}
                </div>
            )}
        </section>
    )
};

export default Collections;