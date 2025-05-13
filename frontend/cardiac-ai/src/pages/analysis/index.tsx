import {
	Box,
	Button,
	FormControlLabel,
	Switch,
	TextField,
	Typography,
	useMediaQuery,
	useTheme,
} from "@mui/material";
import React, { useRef } from "react";

const Analysis = () => {
	const theme = useTheme();
	const canvasRef = useRef<HTMLCanvasElement | null>(null);

	const isMobile = useMediaQuery(theme.breakpoints.down("sm"));
	const isTablet = useMediaQuery(theme.breakpoints.between("sm", "md"));
	const biggerTablet = useMediaQuery(theme.breakpoints.between("md", "lg"));
	const isLaptop = useMediaQuery(theme.breakpoints.between("lg", 1999));
	const isDesktop = useMediaQuery(theme.breakpoints.between(2000, 4000));

	const canvasWidth = isMobile
		? 320
		: isTablet
		? 400
		: biggerTablet
		? 600
		: 800;

	const canvasHeight = isMobile ? 250 : biggerTablet ? 280 : 300;

	React.useEffect(() => {
		const canvas = canvasRef.current;
		if (!canvas) return;

		const ctx = canvas.getContext("2d");
		if (!ctx) return;

		ctx.clearRect(0, 0, canvas.width, canvas.height);

		// Draw a red rectangle
		ctx.fillStyle = "red";
		ctx.fillRect(20, 20, 100, 100);
	}, []);
	return (
		<Box
			sx={{
				width: "100vw",
				height: "calc(100vh - 4rem)",
				display: "flex",
				flexDirection: "column",
				justifyContent: "center",
				alignItems: "center",
			}}
		>
			<Box
				sx={{
					background: "rgba(249, 250, 255, 1)",
					// width: { xs: "80%", sm: "70%" },
					// height: "70%",
					width: "calc(100% - 6rem)",
					height: "calc(100% - 6rem)",
					display: "flex",
					flexDirection: "column",
					padding: "3rem",
					borderRadius: "1rem",
				}}
			>
				<Box
					sx={{
						display: "flex",
						flexDirection: "row",
						justifyContent: "space-between",
						alignItems: "center",
					}}
				>
					<FormControlLabel
						control={<Switch defaultChecked sx={{ zIndex: 1 }} />}
						label="Save Data"
					/>
					<Typography>Device ID: 123456789</Typography>
					<Typography>Session: 01:23</Typography>
					<Typography>1000 data points: 39 KB</Typography>
				</Box>
				<Box sx={{ width: "100%", margin: "1rem 0" }}>
					<canvas
						ref={canvasRef}
						width={canvasWidth}
						height={canvasHeight}
						style={{
							border: "1px solid #000",
							borderRadius: "1rem",
							background: "#fff",
						}}
					/>
				</Box>
				<Box
					sx={{
						display: "flex",
						flexDirection: { xs: "column", sm: "row" },
						justifyContent: "space-evenly",
						padding: "1.5rem",
						gap: "1rem",
					}}
				>
					<Button
						variant="outlined"
						sx={{
							border: "1px solid rgba(47, 90, 196, 1)",
							padding: "1rem 2rem",
							background: "#fff",
							borderRadius: "1rem",
							":hover": {
								border: "2px solid rgba(47, 90, 196, 1)",
								background: "rgba(47, 90, 196, 1)",
								color: "#fff",
							},
						}}
					>
						Download Previous
					</Button>
					<Button
						variant="outlined"
						sx={{
							border: "1px solid rgba(47, 90, 196, 1)",
							padding: "1rem 2rem",
							background: "#fff",
							borderRadius: "1rem",
							":hover": {
								border: "2px solid rgba(47, 90, 196, 1)",
								background: "rgba(47, 90, 196, 1)",
								color: "#fff",
							},
						}}
					>
						Download Current
					</Button>
					<Button
						variant="outlined"
						sx={{
							border: "1px solid rgba(47, 90, 196, 1)",
							padding: "1rem 2rem",
							background: "#fff",
							borderRadius: "1rem",
							":hover": {
								border: "2px solid rgba(47, 90, 196, 1)",
								background: "rgba(47, 90, 196, 1)",
								color: "#fff",
							},
						}}
					>
						Request AI Analysis
					</Button>
				</Box>
				<Box sx={{ padding: { xs: "0", sm: "1.5rem" } }}>
					<TextField
						placeholder="AI Analysis will be displayed here..."
						multiline
						rows={5}
						sx={{
							width: { xs: "100%", sm: "90%" },
							background: "#fff",
							"& .MuiInputBase-root": {
								borderRadius: "1rem",
							},
						}}
					/>
				</Box>
			</Box>
		</Box>
	);
};

export default Analysis;
