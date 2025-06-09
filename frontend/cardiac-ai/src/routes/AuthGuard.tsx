import React from "react";
import { Navigate } from "react-router-dom";
import { useAuth } from "../context/AuthProvider";
import { Box, CircularProgress } from "@mui/material";
import Layout from "../layout";

interface Props {
	redirectPath?: string;
	children: React.ReactElement;
}

const AuthGuard: React.FC<Props> = ({ redirectPath = "/login", children }) => {
	const { isAuthenticated, loading, verifyToken } = useAuth();

	console.log("isAuthenticated: ", isAuthenticated);
	console.log("loading: ", loading);

	React.useEffect(() => {
		verifyToken();
	}, []);

	if (loading) {
		return (
			<Layout>
				<Box
					sx={{
						display: "flex",
						justifyContent: "center",
						alignItems: "center",
						height: "100vh",
					}}
				>
					<CircularProgress size={'5rem'} />
				</Box>
			</Layout>
		);
	}

	if (!isAuthenticated) {
		return <Navigate to={redirectPath} replace />;
	}

	return children;
};

export default AuthGuard;
